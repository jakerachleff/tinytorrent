//
// Created by Jake Rachleff on 11/25/17.
//

#ifndef TINYTORRENT_TCPCLIENT_H
#define TINYTORRENT_TCPCLIENT_H

#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class tcpclient {
public:
    tcpclient(boost::asio::io_service& io_service, std::string ip_addr, unsigned short port)
        : socket_(io_service), endpoint_(boost::asio::ip::address::from_string("127.0.0.1"), 12345)
    {
        m_pWork.reset(new boost::asio::io_service::work(io_service));
    }

    void request_song(std::string id)
    {
        song_id_ = id;
        boost::system::error_code error = boost::asio::error::host_not_found;


        std::cout << "About to connect to socket" << std::endl;
        socket_.connect(endpoint_, error);


        if (error)
            throw boost::system::system_error(error);

        socket_.async_read_some(boost::asio::buffer(buf),
                                boost::bind(&tcpclient::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));

        boost::asio::async_write(socket_, boost::asio::buffer(id),
                                 boost::bind(&tcpclient::handle_write, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));



        std::cout << "Client: Successfully connected to socket" << std::endl;
    }
private:
    void handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(buf),
                                    boost::bind(&tcpclient::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));

            boost::asio::async_write(socket_, boost::asio::buffer(song_id_),
                                     boost::bind(&tcpclient::handle_write, this,
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
        }
        else socket_.close();
    }

    void handle_write(const boost::system::error_code& error,
                      size_t bytes_transferred)
    {
        std::cout << "Client: Wrote " << bytes_transferred << " bytes to server." << std::endl;
    }

    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error)
        {
            std::cout << "Client: Read " << buf.data() << " from server." << std::endl;
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(buf),
                                    boost::bind(&tcpclient::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cout << "Error Occurred" << std::endl;
            socket_.close();
        }
    }


    boost::shared_ptr<boost::asio::io_service::work> m_pWork;
    std::string song_id_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    boost::array<char, 1024> buf;
};


#endif //TINYTORRENT_TCPCLIENT_H
