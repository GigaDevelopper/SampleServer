#include "server/connection.h"
#include "utils/utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>


server::http_connection::http_connection(asio::ip::tcp::socket socket, context_ptr &io_context, const std::string &root_directory, int cach_size, cache_ptr &cache)
    :socket_(std::move(socket)), strand_(io_context->get_executor()), root_directory_{root_directory}, cache_size_{cach_size}
{
    images_cache_ = cache;
}

void server::http_connection::start()
{
    read_requests();
}

void server::http_connection::read_requests() {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, request_buf_, "\r\n",
                                  boost::bind(&server::http_connection::handle_request, this, boost::asio::placeholders::error, self));
}

void server::http_connection::write_response(std::string&& response_str) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(std::move(response_str)),
                             [this, self](boost::system::error_code ec, std::size_t /*bytes_transferred*/) {
                                 if (!ec) {
                                     boost::system::error_code ignored_ec;
                                     socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
                                 }
                             });
}

void server::http_connection::handle_request(const boost::system::error_code& ec, std::shared_ptr<server::http_connection> self) {
    if (!ec) {
        std::istream request_stream(&request_buf_);
        std::string request_method;
        std::string request_path;
        std::string http_version;
        request_stream >> request_method >> request_path >> http_version;
        std::clog << "Path: " << request_path << "  Method: "<<request_method << std::endl;

        // Check request method and path
        if (request_method == "GET" && request_path.rfind("/image/", 0) == 0) {
            get_image_handler(request_path);
        }
        else
            if (request_method == "GET" && (request_path == "/images" || request_path == "/images/")) {
                get_images_handler();
            }
            else {
                server::http::http_response response{
                    "HTTP/1.1 400 Bad Request",
                    {{"Content-Type", "text/plain"},
                     {"Content-Length", "0"},
                     {"Connection", "close"}},
                    ""
                };
                write_response(response.to_string());
            }
    }
}

void server::http_connection::get_image_handler(const std::string& request_path) {
    std::string image_name = request_path.substr(7); // Remove "/image/"
    if (server::utils::fs::extension(root_directory_ +"/" + image_name) != ".png"){
        server::http::http_response response{
            "HTTP/1.1 400 Bad Request",
            {{"Content-Type", "text/plain"},
             {"Content-Length", "0"},
             {"Connection", "close"}},
            ""
        };
        write_response(response.to_string());
        return;
    }

    std::string cached_image = images_cache_->get(image_name);
    //check cached image
    if (!cached_image.empty()) {
        server::http::http_response response{
            "HTTP/1.1 200 OK",
            {{"Content-Type", "image/png"},
             {"Content-Length", std::to_string(cached_image.size())},
             {"Connection", "close"}},
            cached_image
        };
        write_response(response.to_string());
    }
    else {
        std::string image_content = server::utils::read_file(root_directory_ + "/" + image_name);
        //if image found
        if (!image_content.empty()) {
            images_cache_->put(image_name, image_content);
            server::http::http_response response{
                "HTTP/1.1 200 OK",
                {{"Content-Type", "image/png"},
                 {"Content-Length", std::to_string(image_content.size())},
                 {"Connection", "close"}},
                image_content
            };
            write_response(response.to_string());
        }
        //else not found
        else {
            server::http::http_response response{
                "HTTP/1.1 404 Not Found",
                {{"Content-Type", "text/plain"},
                 {"Content-Length", "0"},
                 {"Connection", "close"}},
                ""
            };
            write_response(response.to_string());
        }
    }
}

void server::http_connection::get_images_handler() {
    std::vector<std::string> image_names;
    for (auto it = images_cache_->begin(); it != images_cache_->end(); ++it) {
        image_names.push_back(*it);
    }

    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < image_names.size(); ++i) {
        oss << "\"" << image_names[i] << "\"";
        if (i != image_names.size() - 1) {
            oss << ",";
        }
    }
    oss << "]";

    std::string images_list = oss.str();
    server::http::http_response response{
        "HTTP/1.1 200 OK",
        {{"Content-Type", "application/json"},
         {"Content-Length", std::to_string(images_list.length())},
         {"Connection", "close"}},
        images_list
    };
    write_response(response.to_string());
}
