#ifndef HATH_HTTPRESPONSE_H
#define HATH_HTTPRESPONSE_H

#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "util.h"

class HTTPResponse : public std::enable_shared_from_this<HTTPResponse>
{
public:
    HTTPResponse();

    std::shared_ptr<HTTPResponse> begin(uint16_t code = 200, std::string custom_reason = "");
    std::shared_ptr<HTTPResponse> header(std::string field, std::string value);
    std::shared_ptr<HTTPResponse> write(const std::vector<char> &data);
    std::shared_ptr<HTTPResponse> write(const std::string &data);
    void end();

    std::vector<char> toHTTP();

    uint16_t code;
    std::string reason;
    std::map<std::string, std::string, ci_less> headers;
    std::vector<char> body;
};

#endif //HATH_HTTPRESPONSE_H
