#include "server/http_server.h"
#include "utils/lrucache.h"
#include "utils/utils.h"

#include <boost/asio/io_context.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using server::utils::lrucache;

int main(int argc, char** argv)
{
    // server::utils::command_line_ptions options;
    // if(options(argc, argv))
    // {
    //
        server::http_server server("/home/azmiddin", 4, 4080);
        server.run();

    return 0;
}
