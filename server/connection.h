#ifndef CONNECTION_H
#define CONNECTION_H

#include "utils/lrucache.h"

#include <boost/asio.hpp>
#include <string>

namespace server {

namespace asio = boost::asio;
using context_ptr = std::shared_ptr<boost::asio::io_context>;
using cache_ptr = std::shared_ptr<server::utils::lrucache>;


class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    http_connection(asio::ip::tcp::socket socket, context_ptr& io_context,
                    const std::string & root_directory, int cach_size, cache_ptr &cache);
    void start();

private:
    void read_requests();
    void write_response(std::string && response);

    //handlers
    void handle_request(const boost::system::error_code& ec, std::shared_ptr<server::http_connection> self);
    void get_image_handler(const std::string& request_path);
    void get_images_handler();

private:
    asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    asio::streambuf request_buf_;

    std::string root_directory_;
    int cache_size_;

    cache_ptr images_cache_;
};

} // namespace server


#endif // CONNECTION_H
