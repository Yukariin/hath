#ifndef HATH_HTTPCONNECTION_H
#define HATH_HTTPCONNECTION_H

#include <array>
#include <memory>

#include "asio.hpp"
#include "HTTPParser.h"
#include "HTTPHandler.h"

using namespace asio::ip;

class HTTPConnectionManager;

class HTTPConnection
    : public std::enable_shared_from_this<HTTPConnection>
{
public:
    HTTPConnection(tcp::socket socket, HTTPConnectionManager &manager, HTTPHandler &handler);

    void read();
    void write(std::vector<char> data);
    void start();
    void stop();

private:
    tcp::socket socket;
    std::array<char, 8192> buffer;
    HTTPConnectionManager& manager;
    HTTPHandler& handler;
    HTTPParser parser;
};

typedef std::shared_ptr<HTTPConnection> connectionPtr;

#endif //HATH_HTTPCONNECTION_H
