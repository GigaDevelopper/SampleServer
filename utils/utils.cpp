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

std::string server::utils::get_images_list(const std::string &path)
{
    std::ostringstream oss;
    oss << "{ \"images\": [";

    try {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Invalid path or not a directory");
        }

        bool first = true;
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry) && entry.path().extension() == ".png") {

                if (!first) {
                    oss << ", ";
                }
                first = false;

                std::string filename = entry.path().filename().string();
                std::vector<unsigned char> file_data = read_file(entry.path().string());
                std::string base64_data = base64_encode(file_data);

                oss << "{ \"name\": \"" << filename << "\", \"image\": \"" << base64_data << "\" }";
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return "{}";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "{}";
    }

    oss << "] }";
    return oss.str();
}


std::string server::utils::base64_encode(const std::vector<unsigned char> &data)
{
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string base64_str;
    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            base64_str.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        base64_str.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (base64_str.size() % 4) {
        base64_str.push_back('=');
    }
    return base64_str;
}

std::vector<unsigned char> server::utils::read_file(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return buffer;
}
