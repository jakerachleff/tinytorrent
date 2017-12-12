//
// Created by Jake Rachleff on 11/24/17.
// Based on Tutorials from Boost Documentation
//

#ifndef TINYTORRENT_TCPCONNECTION_H
#define TINYTORRENT_TCPCONNECTION_H


#include <deque>
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;


using namespace boost::asio::ip;

#define MESSAGE_SIZE 4096

struct file_message {
    char data[MESSAGE_SIZE];
    size_t len;
};

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
        socket_.async_read_some(boost::asio::buffer(read_buf),
                                boost::bind(&tcpconnection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

private:
    tcpconnection(boost::asio::io_service& io_service)
            : socket_(io_service)
    {
        std::cout << "New TCP Connection Created" << std::endl;
        read_buf.assign(0);
    }

    /* MUST HAVE write_queue_lock_ WHEN CALLING THIS FUNCTION. */
    void do_write()
    {

        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_queue_.front().data,
                                                     write_queue_.front().len),
                                 boost::bind(&tcpconnection::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

    void queue_file_to_write(std::string filename)
    {
        filename = "/home/jestjest/senior/cs244b/CLionProjects/tinytorrent/" + filename;
        std::ifstream filestream(filename);

        while (true)
        {
            /* Read chunk of file into fm. */
            file_message fm;
            filestream.read(fm.data, MESSAGE_SIZE);
            fm.len = filestream.gcount();

            /* Write chunk of file into write queue. */
            write_queue_lock_.lock();
            bool write_in_progress = !write_queue_.empty();
            write_queue_.push_back(fm);
            if (!write_in_progress) do_write();
            write_queue_lock_.unlock();
            if (filestream.eof()) break;
        }
    }


    void handle_write(const boost::system::error_code& error,
                      size_t bytes_transferred)
    {
        if (!error)
        {
            write_queue_lock_.lock();
            write_queue_.pop_front();
            if (!write_queue_.empty())
            {
                do_write();
            }
            write_queue_lock_.unlock();
        }
        else
        {
            std::cout << "Server: Error occured in handle_read: " << error.message() << " " << error.value() << std::endl;
            std::cout << socket_.is_open() << std::endl;
        }

    }

    void handle_read(const boost::system::error_code& error,
                      size_t bytes_transferred)
    {
        if (!error)
        {
            std::cout << "Server: Read " << read_buf.data() << " from client." << std::endl;
            std::string filename = "files/" + std::string(read_buf.data());

            auto self(shared_from_this());
            std::thread t;
            t = std::thread([this, self, filename]() {
                queue_file_to_write(filename);
            });
            t.detach();
        }
        else
        {
            std::cout << "Server: Error occured in handle_read: " << error.message() << " " << error.value() << std::endl;
            std::cout << socket_.is_open() << std::endl;
        }

    }

    std::mutex write_queue_lock_;
    std::deque<file_message> write_queue_;
    tcp::socket socket_;
    std::string item_id_;
    boost::array<char, 1024> read_buf;
    char write_buf[4096];
};

#endif //TINYTORRENT_TCPCONNECTION_H
