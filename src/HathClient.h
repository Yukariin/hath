#ifndef HATH_HATHCLIENT_H
#define HATH_HATHCLIENT_H

#include "Cache.h"
#include "ServerAPI.h"
#include "HTTPServer.h"

class HathClient
{
public:
    static HathClient& instanse();

    HathClient(HathClient const&) = delete;
    HathClient(HathClient&&) = delete;
    HathClient& operator=(HathClient const&) = delete;
    HathClient& operator=(HathClient &&) = delete;

    int run(int argc, char *argv[]);
    static void dieWithError(std::exception &e);
    static void dieWithError(std::string error);

    std::shared_ptr<ServerAPI> getServerAPI();
    std::shared_ptr<Cache> getCache();
    std::shared_ptr<HTTPServer> getServer();

private:
    std::shared_ptr<ServerAPI> serverAPI;
    std::shared_ptr<Cache> cache;
    std::shared_ptr<HTTPServer> server;

    HathClient();

    void shutdown(std::string s);
    static void shutdownHook(int signum);
};

#endif //HATH_HATHCLIENT_H
