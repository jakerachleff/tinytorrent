#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>

#include "include/tcpclient.hpp"
#include "include/tcpserver.hpp"
#include "include/ThreadPool.h"

#include "simple-kademlia/include/node/kademlianode.hpp"

void download_item(std::string item_id, bool production_mode, std::string ip_addr, short port)
{
    boost::asio::io_service client_io_service;
    tcpclient client(client_io_service, ip_addr, port);

    std::thread clientIoThread;
    clientIoThread = std::thread([&client_io_service]() {
        client_io_service.run();
        std::cout << "Client: Shutting down..." << std::endl;
    });

    client.request_song(item_id, production_mode);
    std::cout << "Main Thread: Request for " + item_id + " complete." << std::endl;

    client_io_service.stop();
    if(clientIoThread.joinable())
        clientIoThread.join();
}

void run_server()
{
    boost::asio::io_service io_service;
    tcpserver server(io_service, 12345);
    std::cout << "Launching TCP Server" << std::endl;

    std::thread ioThread;
    ioThread = std::thread([&io_service]() {
        io_service.run();
        std::cout << "Server: Shutting down..." << std::endl;
    });

    std::cout << "Launched TCP Server, Please Hit Enter to Shut Down Server" << std::endl;
    std::string enter_line;
    std::getline(std::cin, enter_line);

    io_service.stop();
    if (ioThread.joinable())
        ioThread.join();
}

void run_client(bool production_mode)
{
    /* For demo use, please set the IP address here, and then again in nodeinfo.hpp. */
    kdml::KademliaNode node("10.34.105.71", 8000);
    ThreadPool pool(std::thread::hardware_concurrency());
    std::cout << "Please Request a File:" << std::endl;

    std::string command;
    while (std::getline(std::cin, command)) {

        node.get(command, [&pool, command, production_mode](kdml::Nodes peers){
            pool.enqueue(download_item, command, production_mode, peers[0].getIpAddr(), peers[0].port);
        });

        std::cout << command << std::endl;
        if (command.size() == 0) break;

    }

}

int main(int argc, const char *argv[]) {
    try
    {
        namespace po = boost::program_options;
        int opt;
        po::options_description desc("Options");
        desc.add_options()
                ("server", po::value<bool>()->default_value(false), "Run server")
                ("production", po::value<bool>()->default_value(false), "Run server under production settings");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }

        bool production_mode = vm.count("production") && vm["production"].as<bool>();

        if (vm.count("server") && vm["server"].as<bool>() ) run_server();
        else run_client(production_mode);

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}