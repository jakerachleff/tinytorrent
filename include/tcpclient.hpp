//
// Created by Jake Rachleff on 11/25/17.
//

#ifndef TINYTORRENT_TCPCLIENT_H
#define TINYTORRENT_TCPCLIENT_H

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class tcpclient {
public:

    tcpclient(boost::asio::io_service& io_service, std::string ip_addr, unsigned short port)
        : socket_(io_service), endpoint_(boost::asio::ip::address::from_string(ip_addr), port)
    {
        m_pWork.reset(new boost::asio::io_service::work(io_service));
    }

    bool request_song(std::string id, bool production_mode)
    {
        boost::system::error_code connect_error = boost::asio::error::host_not_found;

        std::string file_directory = production_mode ? "files/" : "clientfiles/";
        std::string output_filename = file_directory + id;
        std::cout << "Client output filename is: " << output_filename << std::endl;
        std::ofstream output_filestream;
        output_filestream.open(output_filename);

        std::cout << "About to connect to socket with IP: " << endpoint_.address()
                  << " and port: " << endpoint_.port() << std::endl;
        socket_.connect(endpoint_, connect_error);


        if (connect_error)
        {
            std::cout << "Client: Could not connect to socket with IP: "
                      << endpoint_.address() << " and port " << endpoint_.port() << std::endl;
            return false;
        }
        std::cout << "Client: Successfully connected to socket" << std::endl;


        boost::array<char, 4096> buf;
        boost::system::error_code err;

        size_t len = socket_.write_some(boost::asio::buffer(id), err);
        if (err) throw boost::system::system_error(err);

        size_t total_len = 0;
        for (;;)
        {
            len = socket_.read_some(boost::asio::buffer(buf), err);
            total_len += len;
            output_filestream.write(buf.data(), len);
            if (err == boost::asio::error::eof)
            {
                std::cout << "Client: Download complete, read " << total_len << " bytes from server. Shutting down..." << std::endl;
                break; // Connection closed cleanly by peer.
            }
            else if (err)
                throw boost::system::system_error(err); // Some other error.

        }
        output_filestream.close();
        return true;
    }

private:
    boost::shared_ptr<boost::asio::io_service::work> m_pWork;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
};


#endif //TINYTORRENT_TCPCLIENT_H
