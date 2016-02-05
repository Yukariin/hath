#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(std::string method, std::string url, header_map headers, std::vector<char> body, bool upgrade)
    : method(method), url(url), headers(headers), body(body), upgrade(upgrade)
{

}

std::shared_ptr<HTTPRequest> HTTPRequest::header(std::string field, std::string value)
{
    headers.insert(std::pair<std::string, std::string>(field, value));

    return shared_from_this();
}

std::vector<char> HTTPRequest::toHTTP()
{
    std::stringstream ss;

    // Method and path
    ss << method << " " << url.pathAndQuery() << " HTTP/1.1\r\n";

    // Generate the Date header on the fly
    ss << "Host: " << url.host << "\r\n";

    // Headers
    for(auto it : headers)
        ss << it.first << ": " << it.second << "\r\n";

    // A blank line terminates the header section
    ss << "\r\n";

    std::string headers_str = ss.str();
    std::vector<char> data(headers_str.begin(), headers_str.end());

    data.insert(data.end(), body.begin(), body.end());

    return data;
}
