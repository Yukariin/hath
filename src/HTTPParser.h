#ifndef HATH_HTTPPARSER_H
#define HATH_HTTPPARSER_H

#include <string>
#include <memory>

#include "http_parser.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTPParser
{
public:
    enum Mode {
        Request,
        Response
    };

    HTTPParser(Mode mode);
    ~HTTPParser();

    enum Status {
        Error = -1,
        KeepGoing = 0,
        GotRequest = 1
    };

    Status parseChunk(const char *data, std::size_t length);

    std::shared_ptr<HTTPRequest> req;
    std::shared_ptr<HTTPResponse> res;

    bool done = false;
    bool was_reading_header_value = false;
    std::string tmp_header_field;
    std::string tmp_header_value;

private:
    http_parser *parser;
    http_parser_settings parser_settings;
};

#endif //HATH_HTTPPARSER_H
