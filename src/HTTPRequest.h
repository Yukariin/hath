#ifndef HATH_HTTPREQUEST_H
#define HATH_HTTPREQUEST_H

#include <string>
#include <map>
#include <vector>

#include "util.h"

class HTTPRequest {
public:
    typedef std::map<std::string, std::string, ci_less> header_map;

    HTTPRequest(std::string method = "", std::string url = "", header_map header = header_map{}, std::vector<char> body = {}, bool upgrade = false);
    virtual ~HTTPRequest();

    std::string method;
    std::string url;
    std::map<std::string, std::string, ci_less> header;
    std::vector<char> body;

    bool upgrade;
};

#endif //HATH_HTTPREQUEST_H
