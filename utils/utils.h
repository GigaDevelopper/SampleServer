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

std::string get_images_list(const std::string& path);
std::string base64_encode(const std::vector<unsigned char>& data);
std::vector<unsigned char> read_file(const std::string& path);

}
}
#endif // UTILS_H
