#ifndef HATH_HTTPSERVER_H
#define HATH_HTTPSERVER_H

#include <deque>
#include <memory>
#include <thread>

#include "asio.hpp"

#include "HTTPConnection.h"
#include "HTTPConnectionManager.h"
#include "HTTPHandler.h"

using namespace asio;
using namespace asio::ip;

class HTTPServer
{
public:
    HTTPServer(const HTTPServer&) = delete;
    HTTPServer& operator=(const HTTPServer&) = delete;

    explicit HTTPServer(unsigned int workers);
    virtual ~HTTPServer();

    asio::error_code listen(const tcp::endpoint &endpoint);
    asio::error_code listen(const std::string &address, const uint16_t &port);
    asio::error_code listen(const uint16_t &port);
    void run();
    void stop();

protected:
    void accept();

private:
    io_service service;
    io_service::work work;
    std::deque<std::thread> worker_threads;
    tcp::acceptor acceptor;
    std::shared_ptr<HTTPConnection> newConnection;
    HTTPConnectionManager manager;
    HTTPHandler request_handler;
};

#endif //HATH_HTTPSERVER_H
