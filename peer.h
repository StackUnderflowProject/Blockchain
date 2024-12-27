//
// Created by mic on 12/26/24.
//

#ifndef PEER_H
#define PEER_H

#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <vector>

using boost::asio::ip::tcp;

class peer : public std::enable_shared_from_this<peer> {
public:
    explicit peer(tcp::socket socket): socket_(std::move(socket)) {
    }

    void start() {
        do_read();
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this, self](const boost::system::error_code &ec, const std::size_t length) {
                if (!ec) {
                    // Print the received message
                    std::cout << "Received: " << std::string(data_, length) << std::endl;

                    // Clear the buffer to prevent stale data from being reprocessed
                    std::fill(data_, data_ + max_length, 0);

                    // Respond to the client or take action
                    do_write("Message received!");

                    // Continue reading for new messages
                    do_read();
                } else if (ec != boost::asio::error::eof) {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
            });
    }

    void do_write(const std::string &response) {
        auto self(shared_from_this());
        async_write(
            socket_, boost::asio::buffer(response),
            [this, self](const boost::system::error_code &ec, std::size_t /*length*/) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
    }
};


#endif //PEER_H
