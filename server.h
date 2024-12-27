//
// Created by mic on 12/26/24.
//

#ifndef SERVER_H
#define SERVER_H

#include "peer.h"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>


using boost::asio::ip::tcp;
using boost::system::error_code;

class server {
public:
    server(boost::asio::io_context &io_context, const short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    tcp::acceptor acceptor_;

    void do_accept() {
        acceptor_.async_accept(
            [this](const error_code &ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<peer>(std::move(socket))->start();
                }
                do_accept();
            });
    }
};


#endif //SERVER_H
