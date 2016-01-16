#ifndef HATH_HTTPSERVER_H
#define HATH_HTTPSERVER_H

#include <deque>
#include <memory>

#include "asio.hpp"
#include "HTTPConnection.h"
#include "HTTPConnectionManager.h"

#define BufferSize 1024

using namespace asio;
using namespace asio::ip;

class HTTPServer
{
public:
    HTTPServer(const HTTPServer&) = delete;
    HTTPServer& operator=(const HTTPServer&) = delete;

    HTTPServer(unsigned int workers);
    virtual ~HTTPServer();

    asio::error_code listen(const tcp::endpoint &endpoint);
    asio::error_code listen(const std::string &address, const uint16_t &port);
    asio::error_code listen(const uint16_t &port);
    std::thread && run();
    void stop();

protected:
    void accept();

private:
    io_service service;
    io_service::work work;
    std::deque<std::thread> worker_threads;
    tcp::acceptor acceptor;
    tcp::socket socket;
    HTTPConnectionManager manager;
};

#endif //HATH_HTTPSERVER_H
