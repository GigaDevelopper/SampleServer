#include "server/connection.h"
#include "utils/utils.h"

#include <fstream>
#include <sstream>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>


server::http_connection::http_connection(asio::ip::tcp::socket socket, context_ptr &io_context, const std::string &root_directory, int cach_size)
    :socket_(std::move(socket)), strand_(io_context->get_executor()), root_directory_{root_directory}, cache_size_{cach_size}
{ }

void server::http_connection::start()
{
    read_requests();
}

void server::http_connection::read_requests()
{
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, request_buf_, "\r\n",
                                  [this, self](boost::system::error_code ec, std::size_t /*bytes_transferred*/) {
                                      if (!ec) {
                                          std::istream request_stream(&request_buf_);
                                          std::string request_method;
                                          std::string request_path;
                                          std::string http_version;
                                          request_stream >> request_method >> request_path >> http_version;

                                          if (request_method == "GET" && request_path == "/") {
                                              std::string response =
                                                 "HTTP/1.1 200 OK\r\n"
                                                 "Content-Type: text/plain\r\n"
                                                 "Content-Length: 4\r\n"
                                                 "Connection: close\r\n"
                                                  "\r\n"
                                                  "Hello";
                                              write_response(std::move(response));
                                          } else if (request_method == "GET" && request_path == "/images/") {
                                              std::string images_list = server::utils::get_images_list("/home/azmiddin");
                                              std::string response =
                                                  "HTTP/1.1 200 OK\r\n"
                                                  "Content-Type: application/json\r\n"
                                                  "Content-Length: " + std::to_string(images_list.length()) + "\r\n"
                                                                                           "Connection: close\r\n"
                                                                                           "\r\n" +
                                                  images_list;
                                              write_response(std::move(response));
                                          } else {
                                              std::string response =
                                                  "HTTP/1.1 405 Method Not Allowed\r\n"
                                                  "Content-Type: text/plain\r\n"
                                                  "Content-Length: 0\r\n"
                                                  "Connection: close\r\n"
                                                  "\r\n";
                                              write_response(std::move(response));
                                          }
                                      }
                                  });
}

void server::http_connection::write_response(std::string &&response)
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(std::move(response)),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                 if (!ec) {
                                     boost::system::error_code ignored_ec;
                                     socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
                                 }
                             });
}
