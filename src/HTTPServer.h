#ifndef HATH_HTTPSERVER_H
#define HATH_HTTPSERVER_H

#include "asio.hpp"

using namespace asio;
using namespace asio::ip;

class HTTPServer {
public:
    HTTPServer();
    virtual ~HTTPServer();

    asio::error_code listen(const tcp::endpoint &endpoint);
    asio::error_code listen(const std::string &address, const uint16_t &port);
    asio::error_code listen(const uint16_t &port);
    void run();
    void stop();

private:
    io_service service;

    tcp::acceptor acceptor;
};

#endif //HATH_HTTPSERVER_H
