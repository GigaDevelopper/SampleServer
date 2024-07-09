#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <vector>

namespace server {

namespace asio = boost::asio;

class HTTPServer {
public:
    HTTPServer(const std::string& root_directory, int cache_size, unsigned short port);

    void run();

private:
    void start_accept();

private:
    std::unique_ptr<asio::io_context> io_context_;
    std::vector<std::thread> workers_;
    std::unique_ptr<asio::io_context::work> work_;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;

    std::string root_directory_;
    int cache_size_;
    unsigned short port_;
};

} // namespace server

#endif // HTTP_SERVER_H
