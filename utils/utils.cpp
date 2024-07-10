#include "utils.h"

#include <boost/filesystem.hpp>
#include <iostream>

bool server::utils::command_line_ptions::operator()(int argc, char **argv)
{
    try {
        po::options_description desc("Allowed options ");
        desc.add_options()
            ("help", "help message")
            ("root_directory", po::value<std::string>(&root_directory)->required(), "root directory where images are stored")
            ("cache_size", po::value<int>(&cache_size)->default_value(10), "size of LRU cache");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return false;
        }

        po::notify(vm);

        //check root directory
        if (!boost::filesystem::exists(root_directory) || !boost::filesystem::is_directory(root_directory)) {
            std::cerr << "Error: root directory does not exist or is not a directory" << std::endl;
            return false;
        }

        // check capacity
        if (cache_size <= 0) {
            std::cerr << "Error: cache size must be greater than zero" << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing command line options: " << e.what() << std::endl;
        return false;
    }
}

std::string server::utils::read_file(const std::string &file_path)
{
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return "";
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return std::string(buffer.begin(), buffer.end());
    } else {
        return "";
    }
}

std::string server::http::http_response::to_string() const
{
    std::ostringstream oss;
    oss << status_line << "\r\n";
    for (const auto& header : headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    oss << "\r\n" << body;
    return oss.str();
}
