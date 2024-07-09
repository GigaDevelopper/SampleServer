#include "utils/lrucache.h"
#include <iostream>
#include <utils/utils.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;
using server::utils::lrucache;

int main(int argc, char** argv)
{
    server::utils::command_line_ptions options;
    if(options(argc, argv))
    {
        std::clog <<"All to ok\n";
        std::clog << options.root_directory <<"\n" << options.cache_size;
    }
    return 0;
}
