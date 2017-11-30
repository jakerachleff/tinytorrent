#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>

#include "include/tcpclient.hpp"
#include "include/tcpserver.hpp"

void download_item(std::string item_id)
{
    boost::asio::io_service client_io_service;
    tcpclient client(client_io_service, "127.0.0.1", 12345);

    std::thread clientIoThread;
    clientIoThread = std::thread([&client_io_service]() {
        client_io_service.run();
        std::cout << "Client: Shutting down..." << std::endl;
    });

    client.request_song(item_id);
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

void run_client()
{
    std::vector<std::thread> client_thread_vec;
    std::cout << "Please Request a File:" << std::endl;

    std::string command;
    while (std::getline(std::cin, command)) {
        std::cout << command << std::endl;
        if (command.size() == 0) break;

        client_thread_vec.push_back(std::thread(download_item, command));
    }

    for(auto &client_thread : client_thread_vec) client_thread.join();
}

int main(int argc, const char *argv[]) {
    try
    {
        namespace po = boost::program_options;
        int opt;
        po::options_description desc("Options");
        desc.add_options()
                ("server", po::value<bool>()->default_value(false), "Run server");


        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }

        if (vm.count("server") && vm["server"].as<bool>() ) run_server();
        else run_client();

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}