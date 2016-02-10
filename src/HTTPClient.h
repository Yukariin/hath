#ifndef HATH_HTTPCLIENT_H
#define HATH_HTTPCLIENT_H

#include "asio.hpp"
#include "HTTPResponse.h"

#include <memory>

using namespace asio;
using namespace asio::ip;

class HTTPClient
{
public:
    HTTPClient();

    std::shared_ptr<HTTPResponse> get(std::string url);

private:
    io_service service;
    tcp::resolver resolver;
    tcp::socket socket;
};

#endif //HATH_HTTPCLIENT_H
