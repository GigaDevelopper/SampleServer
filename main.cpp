#include "utils/utils.h"
#include "server/http_server.h"

#include <iostream>
\

int main(int argc, char** argv)
{
    server::utils::command_line_ptions options;

    if(options(argc, argv))
    {
        //try catch for tests
        try{
            server::http_server server(options.root_directory, options.cache_size, 4080);
            server.run();
        }
        catch(const std::exception &e){
            std::cerr <<"Fatal Error: "<< e.what();
        }
    }

    return 0;
}
