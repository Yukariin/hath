#ifndef HATH_HTTPHANDLER_H
#define HATH_HTTPHANDLER_H

#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTPHandler
{
public:
    HTTPHandler();

    void handleRequest(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res);
};

#endif //HATH_HTTPHANDLER_H
