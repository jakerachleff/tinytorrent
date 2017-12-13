#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <sys/wait.h>

#include "include/tcpclient.hpp"
#include "include/tcpserver.hpp"
#include "include/ThreadPool.h"

#include "simple-kademlia/include/node/kademlianode.hpp"


/*
 * Creates a new client connection and downloads an item requested by the Client.
 *
 * @param item_id: string name of the item requested.
 * @param production_mode: bool allows for settings to change between Dev and Prod
 * @param peers: kdml::Nodes A vector of Kademlia NodeInfo objects that Kademlia
 * thinks currently stores the value for item_id
 */
void download_item(const std::string &item_id, bool production_mode, kdml::Nodes peers) {
    for (int i = 0; i < peers.size(); ++i) {
        boost::asio::io_service client_io_service;
        tcpclient client(client_io_service, std::move(peers[i].getIpAddr()), peers[i].port);

        std::thread clientIoThread;
        clientIoThread = std::thread([&client_io_service]() {
            client_io_service.run();
            std::cout << "Client: Shutting down..." << std::endl;
        });

        bool request_complete = client.request_song(item_id, production_mode);

        client_io_service.stop();
        if(clientIoThread.joinable())
            clientIoThread.join();

        if (request_complete) {
            std::cout << "Main Thread: Request for " + item_id + " complete." << std::endl;
            break;
        }
    }
    std::cout << "Main Thread: Request for " + item_id + " unsuccessful." << std::endl;
}

/*
 * Boots up a TinyTorrent Server to serve files to peers.
 *
 * @parm port: short specifying port for server
 *
 */
void run_server(unsigned short port) {
    boost::asio::io_service io_service;
    tcpserver server(io_service, port);
    std::cout << "Launching TCP Server" << std::endl;

    /* Run io_service on separate thread - io_service handles all async calls
     * to our server through Boost's ASIO library. */
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

/*
 * Boots up a TinyTorrent Client, asking the user for put and get commands.
 *
 * @param production_mode: boolean that allows for modifications to
 * TinyTorrent when run in Dev or Prod
 * @param ip: string specifying ip address for client's Kademlia Node
 * @parm port: short specifying port for client's Kademlia Node
 */
void run_client(bool production_mode, std::string ip, unsigned short port) {
    kdml::KademliaNode node(std::move(ip), port);

    /* Please set the IP Address and the Port for the Bootstrap node here. */
    node.bootstrap({"127.0.0.1", 8000});

    /* Create a threadpool for all client download connections. */
    ThreadPool pool(std::thread::hardware_concurrency());
    std::cout << "Type 'put filename' to put a file, 'get filename' to get a file, or hit enter to quit" << std::endl;

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command.find("get ") == 0) {
            std::string song_name = command.substr(4, command.length());
            node.get(song_name, [&pool, song_name, production_mode](kdml::Nodes peers) {
                if (!peers.empty()) pool.enqueue(download_item, song_name, production_mode, peers);
            });
        } else if (command.find("put ") == 0) {
            node.put(command.substr(4, command.length()));
        }

        if (command.empty()) break;
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