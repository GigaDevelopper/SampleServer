#ifndef CONNECTION_H
#define CONNECTION_H

#include "utils/lrucache.h"

#include <boost/asio.hpp>
#include <string>

namespace server {

namespace asio = boost::asio;

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    using pointer = std::shared_ptr<http_connection>;

    static pointer create(asio::io_context& io_context, const std::string& root_directory, int cache_size);
    asio::ip::tcp::socket& socket();
    void start();

private:
    http_connection(asio::io_context& io_context, const std::string& root_directory, int cache_size);
    void read_request();
    void handle_get_request(const std::string& request_line);
    void serve_image(const std::string& image_name);
    void serve_cache_images();
    void write_response(const std::string& response_data);

private:
    asio::ip::tcp::socket socket_;
    asio::streambuf request_buf_;
    std::string root_directory_;
    int cache_size_;
    std::shared_ptr<server::utils::lrucache> image_cache_;
};

} // namespace server


#endif // CONNECTION_H
