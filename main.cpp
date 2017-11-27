#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>

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

            std::thread download_thread = std::thread(download_item, command);
            if (download_thread.joinable()) download_thread.join();
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