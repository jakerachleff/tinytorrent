#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <wait.h>

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

void run_server(unsigned short port)
{
    boost::asio::io_service io_service;
    tcpserver server(io_service, port);
    std::cout << "Launching TCP Server" << std::endl;

    std::thread ioThread;
    ioThread = std::thread([&io_service]() {
        io_service.run();
        std::cout << "Server: Shutting down..." << std::endl;
    });

    std::cout << "Launched TCP Server. Kill client to shut down." << std::endl;
    waitpid(-1, nullptr, 0);

    io_service.stop();
    if (ioThread.joinable())
        ioThread.join();
}

void run_client(bool production_mode, std::string ip, unsigned short port)
{
    /* For demo use, please set the IP address here */
    kdml::KademliaNode node(ip, port);
    node.bootstrap({"127.0.0.1", 8000});

    ThreadPool pool(std::thread::hardware_concurrency());
    std::cout << "Type 'put filename' to put a file, 'get filename' to get a file, or hit enter to quit" << std::endl;

    std::string command;
    while (std::getline(std::cin, command)) {
        /* THIS IS BAD STYLE: 4 IS THE LENGTH OF PUT AND GET. */
        if (command.find("get ") == 0) {
            std::string song_name = command.substr(4, command.length());
            node.get(song_name, [&pool, song_name, production_mode](kdml::Nodes peers){
                pool.enqueue(download_item, song_name, production_mode, peers[0].getIpAddr(), peers[0].port);
            });
        } else if (command.find("put ") == 0) {
            node.put(command.substr(4, command.length()));
        }

        if (command.size() == 0) break;
        std::cout << "Type 'put filename' to put a file, 'get filename' to get a file, or hit enter to quit" << std::endl;
    }
    std::cout << "Done running client" << std::endl;

}

int main(int argc, const char *argv[]) {
    try
    {
        namespace po = boost::program_options;
        int opt;
        po::options_description desc("Options");
        desc.add_options()
                ("ip", po::value<std::string>()->default_value("127.0.0.1"), "IP address")
                ("port", po::value<unsigned short>()->default_value(8000), "Port to send/receive")
                ("production", po::value<bool>()->default_value(false), "Run server under production settings");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }

        bool production_mode = vm.count("production") && vm["production"].as<bool>();

        int pid = fork();

        if (pid == 0) {
            run_client(production_mode, vm["ip"].as<std::string>(), vm["port"].as<unsigned short>());
        } else {
            run_server(vm["port"].as<unsigned short>());
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}