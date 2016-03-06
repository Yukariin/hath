#ifndef HATH_HTTPCONNECTION_H
#define HATH_HTTPCONNECTION_H

#include <array>
#include <memory>

#include "asio.hpp"

#include "HTTPParser.h"
#include "HTTPHandler.h"

using namespace asio;
using namespace asio::ip;

class HTTPConnectionManager;

class HTTPConnection : public std::enable_shared_from_this<HTTPConnection>
{
public:
    HTTPConnection(const HTTPConnection&) = delete;
    HTTPConnection& operator=(const HTTPConnection&) = delete;
    
    explicit HTTPConnection(io_service &service, HTTPConnectionManager &manager, HTTPHandler &handler);

    void start();
    void do_read();
    void do_write(std::vector<char> data);
    void stop();
    tcp::socket& getSocket();

private:
    tcp::socket socket;
    
    std::array<char, 8192> buffer;
    std::array<char, 65536> outbuf;
    
    HTTPConnectionManager& manager;
    HTTPHandler& handler;
    HTTPParser parser;
};

typedef std::shared_ptr<HTTPConnection> connectionPtr;

#endif //HATH_HTTPCONNECTION_H
