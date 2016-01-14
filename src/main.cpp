#include <iostream>

#include "HTTPServer.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello world!" << std::endl;

    HTTPServer server;
    server.listen(8001);
    server.run();

    while (true)
    {

    }

    return 0;
}