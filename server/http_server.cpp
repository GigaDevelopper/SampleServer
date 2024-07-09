#include <server/http_server.h>
#include <server/connection.h>

namespace server {

HTTPServer::HTTPServer(const std::string& root_directory, int cache_size, unsigned short port)
    : root_directory_(root_directory), cache_size_(cache_size), port_(port) {

    this->io_context_ = std::make_unique<asio::io_context>;
    this->work_ = std::make_unique<boost::asio::io_context::work>(io_context_.get());

    const int num_threads = 4;
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([&io_context_]() {
            io_context_->run();
        });
    }

    start_accept();
}

void HTTPServer::run() {
    // В этом месте работа всех контекстов ввода-вывода уже запущена
    for (auto& thread : workers_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void HTTPServer::start_accept() {
    for (size_t i = 0; i < 4; ++i) {
        server::http_connection::pointer new_connection =
            server::http_connection::create(io_contexts_[i], root_directory_, cache_size_);

        auto& acceptor = *acceptors_[i]; // Получаем ссылку на acceptor

        acceptor.async_accept(new_connection->socket(),
                              [this, new_connection, i, &acceptor](const boost::system::error_code& error) {
                                  if (!error) {
                                      new_connection->start();
                                  }
                                  start_accept(); // Перезапускаем ожидание соединений
                              });
    }
}

} // namespace server
