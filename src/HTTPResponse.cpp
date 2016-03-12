#include "HTTPResponse.h"

#include <unordered_map>

static const std::unordered_map<uint16_t, std::string> standard_statuses = {
        {100, "Continue"},
        {101, "Switching Protocols"},
        {102, "Processing"},

        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {203, "Non-Authoritative Information"},
        {204, "No Content"},
        {205, "Reset Content"},
        {206, "Partial Content"},
        {207, "Multi-Status"},							// WebDAV
        {208, "Already Reported"},						// WebDAV
        {226, "IM Used"},								// WebDAV

        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {305, "Use Proxy"},
        {306, "Switch Proxy"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},

        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Request Entity Too Large"},
        {414, "Request-URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Requested Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {418, "I'm a teapot"},							// Best response code
        {419, "Authentication Timeout"},
        {420, "Enhance Your Calm"},						// Nonstandard, used by Twitter
        {422, "Unprocessable Entity"},					// WebDAV
        {423, "Locked"},								// WebDAV
        {424, "Failed Dependency"},						// WebDAV
        {425, "Unordered Collection"},					// WebDAV
        {426, "Upgrade Required"},
        {428, "Precondition Required"},
        {429, "Too Many Requests"},
        {431, "Request Header Fields Too Large"},
        {440, "Login Timeout"},							// Microsoft
        {444, "No Response"},							// Nginx, to ward off malicious request
        {449, "Retry With"},							// Microsoft
        {450, "Blocked by Windows Parental Controls"},	// Microsoft, why
        {451, "Unavailable for Legal Reasons"},
        {494, "Request Header Too Large"},				// Nginx, old status from pre-431

        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"},
        {506, "Variant Also Negotiates"},
        {507, "Insufficient Storage"},					// WebDAV
        {508, "Loop Detected"},							// WebDAV
        {510, "Not Extended"},
        {511, "Network Authentication Required"}
};

HTTPResponse::HTTPResponse()
{

}

std::shared_ptr<HTTPResponse> HTTPResponse::begin(uint16_t code, std::string custom_reason)
{
    if (code != 0)
    {
        this->code = code;
        if (custom_reason.empty())
        {
            auto it = standard_statuses.find(code);
            if (it != standard_statuses.end())
                reason = it->second;
            else
                reason = "???";
        }
    }

    headers.clear();
    body.clear();

    return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::header(std::string field, std::string value)
{
    headers.insert(std::pair<std::string, std::string>(field, value));

    return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::write(const std::vector<char> &data)
{
    body.insert(body.end(), data.begin(), data.end());

    return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::write(const std::string &data)
{
    this->write(std::vector<char>(data.begin(), data.end()));

    return shared_from_this();
}

void HTTPResponse::end()
{
    if (body.size())
    {
        this->header("Cache-Control", "public, max-age=31536000");
        this->header("Content-Length", std::to_string(body.size()));
    }
}

std::vector<char> HTTPResponse::toHTTP()
{
    std::stringstream ss;

    // Status line
    ss << "HTTP/1.1 " << code << " " << reason << "\r\n";

    // Generate the Date header on the fly
    ss << "Date: " << http_date() << "\r\n";

    // Headers
    for(auto &it : headers)
        ss << it.first << ": " << it.second << "\r\n";

    // A blank line terminates the header section
    ss << "\r\n";

    std::string headers_str = ss.str();
    std::vector<char> data(headers_str.begin(), headers_str.end());

    data.insert(data.end(), body.begin(), body.end());

    return data;
}
