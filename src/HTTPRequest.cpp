#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(std::string method, std::string url, header_map header, std::vector<char> body, bool upgrade)
    : method(method), url(url), header(header), body(body), upgrade(upgrade)
{

}

HTTPRequest::~HTTPRequest()
{

}
