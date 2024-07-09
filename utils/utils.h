#ifndef UTILS_H
#define UTILS_H

#include <boost/program_options.hpp>
#include <string>

namespace server {
namespace utils{

namespace  po = boost::program_options;
//struct for parsing command line options
struct command_line_ptions {

    bool operator()(int argc, char**argv);

    std::string root_directory;
    int cache_size;
};
}
}
#endif // UTILS_H
