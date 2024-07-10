#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "utils/lrucache.h"
#include "server/connection.h"

#include <memory>

namespace server {

namespace asio = boost::asio;

class http_server {
public:
    http_server(const std::string& root_directory, int cache_size, unsigned short port);

    void run();

private:
    void start_accept();

private:
    std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;
    std::shared_ptr<asio::io_context> io_context_;

    std::string root_directory_;
    int cache_size_;
    unsigned short port_;

    server::cache_ptr image_cache_;
};

} // namespace server

#endif // HTTP_SERVER_H
