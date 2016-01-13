#ifndef HATH_HTTPRESPONSE_H
#define HATH_HTTPRESPONSE_H

#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include "util.h"

class HTTPResponse
{
public:
    HTTPResponse();
    virtual ~HTTPResponse();

    uint16_t code;
    std::map<std::string, std::string, ci_less> header;
    std::vector<char> body;
};

#endif //HATH_HTTPRESPONSE_H
