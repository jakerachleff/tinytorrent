//
// Created by Jake Rachleff on 11/24/17.
// Based on tutorials from Boost Documentation
//

#ifndef TINYTORRENT_TCPSERVER_H
#define TINYTORRENT_TCPSERVER_H

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "tcpconnection.hpp"

using boost::asio::ip::tcp;

class tcpserver
{
public:
    tcpserver(boost::asio::io_service& io_service, unsigned short port)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        m_pWork.reset(new boost::asio::io_service::work(io_service));
        start_accept();
    }

private:
    void start_accept()
    {
        tcpconnection::pointer new_connection =
                tcpconnection::create(acceptor_.get_io_service());

        acceptor_.async_accept(new_connection->socket(),
                               boost::bind(&tcpserver::handle_accept, this, new_connection,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(tcpconnection::pointer new_connection,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            new_connection->start();
            start_accept();
            std::cout << "Starting to accept connections" << std::endl;
        }
    }

    boost::shared_ptr<boost::asio::io_service::work> m_pWork;
    tcp::acceptor acceptor_;
};


#endif //TINYTORRENT_TCPSERVER_H
