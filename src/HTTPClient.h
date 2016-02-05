#ifndef HATH_HTTPCLIENT_H
#define HATH_HTTPCLIENT_H

#include "asio.hpp"

using namespace asio;
using namespace asio::ip;

class HTTPClient
{
public:
    HTTPClient();

    void get(std::string url);

private:
    io_service service;
    tcp::resolver resolver;
    tcp::socket socket;
};

#endif //HATH_HTTPCLIENT_H
