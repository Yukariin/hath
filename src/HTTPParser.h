#ifndef HATH_HTTPPARSER_H
#define HATH_HTTPPARSER_H

#include <string>
#include <memory>

#include "http_parser.h"
#include "HTTPRequest.h"

class HTTPParser
{
public:
    HTTPParser();
    ~HTTPParser();

    void parse(const char *data, std::size_t length);

private:
    http_parser *parser;
    http_parser_settings parser_settings;

    std::shared_ptr<HTTPRequest> req;
    bool done = false;
    bool was_reading_header_value = false;
    std::string tmp_header_field;
    std::string tmp_header_value;
};

#endif //HATH_HTTPPARSER_H
