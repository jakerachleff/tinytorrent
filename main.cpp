#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "include/tcpclient.hpp"
#include "include/tcpserver.hpp"

int main() {
    try
    {
        boost::asio::io_service io_service;
        tcpserver server(io_service, 12345);
        std::cout << "Launching TCP Server" << std::endl;

        std::thread ioThread;
        ioThread = std::thread([&io_service]() {
            io_service.run();
            std::cout << "Server: Shutting down..." << std::endl;
        });



        std::string command;
        while (std::getline(std::cin, command)) {
            std::cout << command << std::endl;
            if (command.size() == 0) break;


            boost::asio::io_service client_io_service;
            tcpclient client(client_io_service, "127.0.0.1", 12345);

            std::thread clientIoThread;
            clientIoThread = std::thread([&client_io_service]() {
                client_io_service.run();
                std::cout << "Client: Shutting down..." << std::endl;
            });

            client.request_song(command);
            std::cout << "Main Thread: Request for " + command + " complete." << std::endl;

            client_io_service.stop();
            if(clientIoThread.joinable())
                clientIoThread.join();
        }

        io_service.stop();
        if (ioThread.joinable())
            ioThread.join();

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}