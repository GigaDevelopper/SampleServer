#include "server/connection.h"

#include <fstream>
#include <sstream>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>

namespace server {

http_connection::pointer http_connection::create(asio::io_context& io_context, const std::string& root_directory, int cache_size) {
    return pointer(new http_connection(io_context, root_directory, cache_size));
}

asio::ip::tcp::socket& http_connection::socket() {
    return socket_;
}

void http_connection::start() {
    read_request();
}

http_connection::http_connection(asio::io_context& io_context, const std::string& root_directory, int cache_size)
    : socket_(io_context), root_directory_(root_directory), cache_size_(cache_size),
    image_cache_(std::make_shared<server::utils::lrucache>(cache_size)) {}

void http_connection::read_request() {
    auto self(shared_from_this());
    asio::async_read_until(socket_, request_buf_, "\r\n\r\n",
                           [this, self](const boost::system::error_code& error, std::size_t bytes_transferred) {
                               if (!error) {
                                   std::istream request_stream(&request_buf_);
                                   std::string request_line;
                                   std::getline(request_stream, request_line);

                                   if (boost::starts_with(request_line, "GET")) {
                                       handle_get_request(request_line);
                                   } else {
                                       write_response("HTTP/1.1 400 Bad Request\r\n\r\n");
                                   }
                               }
                           });
}

void http_connection::handle_get_request(const std::string& request_line) {
    std::vector<std::string> request_parts;
    boost::split(request_parts, request_line, boost::is_any_of(" "));
    if (request_parts.size() >= 2) {
        std::string path = request_parts[1];

        std::vector<std::string> path_parts;
        boost::split(path_parts, path, boost::is_any_of("/"));
        if (path_parts.size() == 3 && path_parts[1] == "image") {
            serve_image(path_parts[2]);
        } else if (path == "/cache/images") {
            serve_cache_images();
        } else {
            write_response("HTTP/1.1 400 Bad Request\r\n\r\n");
        }
    }
}

void http_connection::serve_image(const std::string& image_name) {
    std::string image_path = root_directory_ + "/" + image_name;
    std::ifstream image_file(image_path, std::ios::binary);
    if (image_file.good()) {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: image/png\r\n";
        response << "Content-Length: " << image_file.tellg() << "\r\n";
        response << "\r\n";
        response << image_file.rdbuf();
        write_response(response.str());
    } else {
        write_response("HTTP/1.1 404 Not Found\r\n\r\n");
    }
}

void http_connection::serve_cache_images() {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: application/json\r\n";
    response << "\r\n";

    std::string json_response = "[";
    for (auto it = image_cache_->begin(); it != image_cache_->end(); ++it) {
        if (it != image_cache_->begin()) {
            json_response += ",";
        }
        json_response += "{\"key\":\"" + *it + "\"}";
    }
    json_response += "]";

    response << json_response;
    write_response(response.str());
}

void http_connection::write_response(const std::string& response_data) {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(response_data),
                      [this, self](const boost::system::error_code& error, std::size_t bytes_transferred) {
                          if (!error) {
                              read_request();
                          }
                      });
}

} // namespace server
