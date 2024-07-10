#ifndef UTILS_H
#define UTILS_H

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <string>

namespace server {
namespace utils{

namespace fs = boost::filesystem;
namespace  po = boost::program_options;

//struct for parsing command line options
struct command_line_ptions {

    bool operator()(int argc, char**argv);

    std::string root_directory;
    int cache_size;
};

std::string read_file(const std::string& file_path);
}//namespace utils

namespace http {
struct http_response {
    std::string status_line;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string to_string() const;
};
}//namespace http utils

}//namespace server
#endif // UTILS_H
