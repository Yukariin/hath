#include <thread>
#include <stdlib.h>

#include "HathClient.h"
#include "Out.h"
#include "Settings.h"
#include "HTTPServer.h"
#include "HTTPClient.h"
#include "ServerAPI.h"

HathClient &HathClient::instanse()
{
    static HathClient myInstance;

    return myInstance;
}

HathClient::HathClient()
{

}

int HathClient::run(int argc, char **argv)
{
    Out o;
    Out::info("Hentai@Home " + Settings::CLIENT_VERSION + " starting up");
    Out::info("");
    Out::info("Copyright (c) 2008-2016, E-Hentai.org, Yukarin - all rights reserved.");
    Out::info("This software comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to modify and redistribute it under the GPL v3 license.");
    Out::info("");

    Settings::parseArgs(argc, argv);

    if (Settings::loadClientLoginFromFile())
        Out::info("Loaded login settings from " + Settings::DATA_FILENAME_CLIENT_LOGIN);

    if (!Settings::loginCredentialsAreSyntaxValid())
        Settings::promptForIDAndKey();

    serverAPI = std::make_shared<ServerAPI>();
    serverAPI->loadClientSettingsFromServer();

    cache = std::make_shared<Cache>();
    cache->initializeCache();
    //cache.flushRecentlyAccessed();

    Out::info("Calculating initial cache list file size...");
    cache->calculateStartupCachedFilesStrlen();
    Out::info("Calculated cacheListStrlen = " + std::to_string(cache->getStartupCachedFilesStrlen()));

    server = std::make_shared<HTTPServer>(1);
    std::error_code ec = server->listen(Settings::getClientPort());
    if (ec)
        dieWithError("Failed to initialize HTTPServer");

    Out::info("Notifying the server that we have finished starting up the client...");
    if (!serverAPI->notifyStart())
    {
        Out::info("Startup notification failed.");
        return 1;
    }

    std::signal(SIGINT, HathClient::shutdownHook);
    std::signal(SIGTERM, HathClient::shutdownHook);

    if (Settings::isWarnNewClient())
        Out::warning("A new client version is available. Please download it from http://hentaiathome.net/ at your convenience.");

    if (cache->getCacheCount() < 1)
    {
        Out::info("Important: Your cache does not yet contain any files.");
        Out::info("Because of this, you won't receive any traffic until the client has downloaded some files.");
        Out::info("This should usually happen within a few minutes, but traffic will take time to build up.");
        Out::info("The longer you run the client, the higher the utilization will become.");
    }

    serverAPI->refreshServerSettings();

    Out::info("Activated.");

    cache->processBlacklist(259200, false);

    int threadSkipCounter = 1;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        if(threadSkipCounter % 30 == 0)
            serverAPI->stillAliveTest();

        if(threadSkipCounter % 2160 == 2159) {
            cache->processBlacklist(43200, false);
        }

        threadSkipCounter++;
    }

    return 0;
}

void HathClient::shutdown(std::string s)
{
    Out::flushLogs();

    Out::info("Shutting down...");

    if (serverAPI != nullptr)
        serverAPI->notifyShutdown();

    if (cache != nullptr)
        //cache->flushRecentlyAccessed();
        cache->terminateDatabase();

    std::vector<std::string> sd = {"I don't hate you", "Whyyyyyyyy...", "No hard feelings", "Your business is appreciated", "Good-night"};
    Out::info(sd[5]);

    Out::disableLogging();

    exit(0);
}

void HathClient::shutdownHook(int /*signum*/)
{
    HathClient::instanse().shutdown("");
}

void HathClient::dieWithError(std::exception &e)
{
    dieWithError(e.what());
}

void HathClient::dieWithError(std::string error)
{
    Out::error("Critical Error: " + error);
    HathClient::instanse().shutdown("");
}

std::shared_ptr<ServerAPI> HathClient::getServerAPI()
{
    return serverAPI;
}

std::shared_ptr<Cache> HathClient::getCache()
{
    return cache;
}

std::shared_ptr<HTTPServer> HathClient::getServer()
{
    return server;
}
