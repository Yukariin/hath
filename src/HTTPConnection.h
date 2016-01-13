#ifndef HATH_HTTPCONNECTION_H
#define HATH_HTTPCONNECTION_H

#include <array>
#include <memory>

#include "asio.hpp"

#define BufferSize 1024

using namespace asio::ip;

class HTTPConnectionManager;

class HTTPConnection
    : public std::enable_shared_from_this<HTTPConnection>
{
public:
    HTTPConnection(tcp::socket socket, HTTPConnectionManager &manager);

    void read();
    void write();
    void start();
    void stop();

private:
    tcp::socket socket;
    HTTPConnectionManager &manager;
    std::array<char, BufferSize> buffer;
};

typedef std::shared_ptr<HTTPConnection> connectionPtr;

#endif //HATH_HTTPCONNECTION_H
