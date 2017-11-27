//
// Created by Jake Rachleff on 11/24/17.
// Based on Tutorials from Boost Documentation
//

#ifndef TINYTORRENT_TCPCONNECTION_H
#define TINYTORRENT_TCPCONNECTION_H


#include <string>
#include <iostream>


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;


using namespace boost::asio::ip;

class tcpconnection
        : public boost::enable_shared_from_this<tcpconnection>
{
public:
    typedef boost::shared_ptr<tcpconnection> pointer;

    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new tcpconnection(io_service));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        boost::asio::async_write(socket_, boost::asio::buffer("ACK"),
                                 boost::bind(&tcpconnection::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));

        socket_.async_read_some(boost::asio::buffer(buf),
                                boost::bind(&tcpconnection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));

    }

private:
    tcpconnection(boost::asio::io_service& io_service)
            : socket_(io_service)
    {
    }

    void handle_write(const boost::system::error_code& error,
                      size_t bytes_transferred)
    {
        if (!error) std::cout << "Server: Wrote " << bytes_transferred << " bytes to client." << std::endl;
        else
        {
            std::cout << "Server: Error occurred in handle_write: " << error.message() << " " << error.value() << std::endl;
        }
    }

    void handle_read(const boost::system::error_code& error,
                      size_t bytes_transferred)
    {
        if (!error)
        {
            std::cout << "Server: Read " << buf.data() << " from client." << std::endl;

//            message_ = "Hello";

//            boost::asio::async_write(socket_, boost::asio::buffer(message_),
//                                     boost::bind(&tcpconnection::handle_write, shared_from_this(),
//                                                 boost::asio::placeholders::error,
//                                                 boost::asio::placeholders::bytes_transferred));

            socket_.async_read_some(boost::asio::buffer(buf),
                                    boost::bind(&tcpconnection::handle_read, shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cout << "Server: Error occured in handle_read: " << error.message() << " " << error.value() << std::endl;
            std::cout << socket_.is_open() << std::endl;
        }

    }


    tcp::socket socket_;
    std::string message_;
    boost::array<char, 1024> buf;
};

#endif //TINYTORRENT_TCPCONNECTION_H
