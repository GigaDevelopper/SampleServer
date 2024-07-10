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

                                          std::clog << request_path << " path\n";

                                          if (request_method == "GET" && request_path.rfind("/image/", 0) == 0) {
                                              std::string image_name = request_path.substr(7); // Remove "/image/"
                                              std::string cached_image = images_cache_->get(image_name);

                                              if (!cached_image.empty()) {
                                                  // Return the cached image
                                                  std::string response =
                                                      "HTTP/1.1 200 OK\r\n"
                                                      "Content-Type: image/png\r\n"
                                                      "Content-Length: " + std::to_string(cached_image.size()) + "\r\n"
                                                                                              "Connection: close\r\n"
                                                                                              "\r\n" +
                                                      cached_image;
                                                  write_response(std::move(response));
                                              } else {
                                                  // Check the image in the file system
                                                  std::string image_path = root_directory_ + "/" + image_name;
                                                  std::ifstream image_file(image_path, std::ios::binary | std::ios::ate);
                                                  if (image_file.is_open()) {
                                                      std::streamsize image_size = image_file.tellg();
                                                      image_file.seekg(0, std::ios::beg);

                                                      std::vector<char> buffer(image_size);
                                                      if (image_file.read(buffer.data(), image_size)) {
                                                          std::string image_content(buffer.begin(), buffer.end());

                                                          // Cache the image
                                                          images_cache_->put(image_name, std::move(image_content));

                                                          std::string response =
                                                              "HTTP/1.1 200 OK\r\n"
                                                              "Content-Type: image/png\r\n"
                                                              "Content-Length: " + std::to_string(buffer.size()) + "\r\n"
                                                                                                "Connection: close\r\n"
                                                                                                "\r\n";
                                                          response.insert(response.end(), buffer.begin(), buffer.end());
                                                          write_response(std::move(response));
                                                      } else {
                                                          // Failed to read the image file
                                                          std::string response =
                                                              "HTTP/1.1 500 Internal Server Error\r\n"
                                                              "Content-Type: text/plain\r\n"
                                                              "Content-Length: 0\r\n"
                                                              "Connection: close\r\n"
                                                              "\r\n";
                                                          write_response(std::move(response));
                                                      }
                                                  } else {
                                                      // Image file not found
                                                      std::string response =
                                                          "HTTP/1.1 404 Not Found\r\n"
                                                          "Content-Type: text/plain\r\n"
                                                          "Content-Length: 0\r\n"
                                                          "Connection: close\r\n"
                                                          "\r\n";
                                                      write_response(std::move(response));
                                                  }
                                              }
                                          } else if (request_method == "GET" && request_path == "/images/") {
                                              // Retrieve list of images from the cache
                                              std::vector<std::string> image_names;
                                              for (auto it = images_cache_->begin(); it != images_cache_->end(); ++it) {
                                                  image_names.push_back(*it);
                                              }

                                              std::string images_list = "[";
                                              for (const auto& name : image_names) {
                                                  images_list += "\"" + name + "\",";
                                              }
                                              if (!image_names.empty()) {
                                                  images_list.pop_back(); // Remove last comma
                                              }
                                              images_list += "]";

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
