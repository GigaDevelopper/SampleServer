#include "server/http_server.h"

#include <iostream>

namespace server {

http_server::http_server(const std::string& root_directory, int cache_size, unsigned short port)
    : root_directory_(root_directory), cache_size_(cache_size), port_(port) {
    io_context_ = std::make_shared<asio::io_context>(4);
    acceptor_ = std::make_shared<asio::ip::tcp::acceptor>(*io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_));
    image_cache_ = std::make_shared<server::utils::lrucache>(cache_size);
}

void http_server::run() {
    std::clog <<"server runingg.. on " << port_ << std::endl;
    start_accept();
    io_context_->run();
}

void http_server::start_accept() {
    acceptor_->async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::clog <<"New incoming connection: ";
                std::make_shared<http_connection>(std::move(socket), io_context_, root_directory_, cache_size_, image_cache_)->start();
            }
            start_accept();
        });
}

} // namespace server
