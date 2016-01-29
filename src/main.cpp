#include <iostream>
#include <thread>
#include <chrono>

#include "HTTPServer.h"
#include "util.h"
#include "Settings.h"

int main(int argc, char *argv[])
{
    Settings::parseArgs(argc, argv);

    if (Settings::loadClientLoginFromFile())
        std::cout << "Loaded from file" << std::endl;

    if (!Settings::loginCredentialsAreSyntaxValid())
        Settings::promptForIDAndKey();

    HTTPServer server(1);
    server.listen(Settings::getClientPort());

    int threadSkipCounter = 1;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        ++threadSkipCounter;
    }

    return 0;
}