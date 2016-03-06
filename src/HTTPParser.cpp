#include "HTTPParser.h"

int request_on_message_begin(http_parser *psr);
int request_on_url(http_parser *psr, const char *data, size_t length);
int request_on_header_field(http_parser *psr, const char *data, size_t length);
int request_on_header_value(http_parser *psr, const char *data, size_t length);
int request_on_headers_complete(http_parser *psr);
int request_on_body(http_parser *psr, const char *data, size_t length);
int request_on_message_complete(http_parser *psr);

int response_on_message_begin(http_parser *psr);
int response_on_status(http_parser *psr, const char *data, size_t length);
int response_on_header_field(http_parser *psr, const char *data, size_t length);
int response_on_header_value(http_parser *psr, const char *data, size_t length);
int response_on_headers_complete(http_parser *psr);
int response_on_body(http_parser *psr, const char *data, size_t length);
int response_on_message_complete(http_parser *psr);


HTTPParser::HTTPParser(Mode mode)
{
    parser = new http_parser;

    if (mode == Request)
    {
        http_parser_init(parser, HTTP_REQUEST);
        parser_settings.on_message_begin = request_on_message_begin;
        parser_settings.on_url = request_on_url;
        parser_settings.on_header_field = request_on_header_field;
        parser_settings.on_header_value = request_on_header_value;
        parser_settings.on_headers_complete = request_on_headers_complete;
        parser_settings.on_body = request_on_body;
        parser_settings.on_message_complete = request_on_message_complete;
    }
    else if (mode == Response)
    {
        http_parser_init(parser, HTTP_RESPONSE);
        parser_settings.on_message_begin = response_on_message_begin;
        parser_settings.on_status = response_on_status;
        parser_settings.on_header_field = response_on_header_field;
        parser_settings.on_header_value = response_on_header_value;
        parser_settings.on_headers_complete = response_on_headers_complete;
        parser_settings.on_body = response_on_body;
        parser_settings.on_message_complete = response_on_message_complete;
    }

    parser->data = this;
}

HTTPParser::~HTTPParser()
{
    delete parser;
}

HTTPParser::Status HTTPParser::parseChunk(const char *data, std::size_t length)
{
    if (done)
    {
        done = false;
        req = nullptr;
    }

    std::size_t parsed = http_parser_execute(parser, &parser_settings, data, length);

    if (parsed != length)
        return Error;

    return (done ? GotRequest : KeepGoing);
}

int request_on_message_begin(http_parser *parser)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->req = std::make_shared<HTTPRequest>();
    return 0;
}

int request_on_url(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    //c->req->url += std::string(dt, l);
    c->req->url = std::string(data, length);
    return 0;
}

int request_on_header_field(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    if (c->was_reading_header_value)
    {
        if (!c->tmp_header_field.empty())
        {
            c->req->headers[c->tmp_header_field] = c->tmp_header_value;
            c->tmp_header_value.clear();
        }

        c->tmp_header_field = std::string(data, length);
        c->was_reading_header_value = false;
    }
    else
    {
        c->tmp_header_field += std::string(data, length);
    }
    return 0;
}

int request_on_header_value(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    if (!c->was_reading_header_value)
    {
        c->tmp_header_value = std::string(data, length);
        c->was_reading_header_value = true;
    }
    else
    {
        c->tmp_header_value += std::string(data, length);
    }
    return 0;
}

int request_on_headers_complete(http_parser *parser)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    if (!c->tmp_header_field.empty())
        c->req->headers[c->tmp_header_field] = c->tmp_header_value;
    return 0;
}

int request_on_body(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->req->body.reserve(c->req->body.size() + length);
    c->req->body.insert(c->req->body.end(), data, data + length);
    return 0;
}

int request_on_message_complete(http_parser *parser)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->req->method = http_method_str(static_cast<http_method>(parser->method));
    c->done = true;
    return 0;
}


int response_on_message_begin(http_parser *parser)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->res = std::make_shared<HTTPResponse>();
    return 0;
}

int response_on_status(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->res->code = static_cast<uint16_t>(parser->status_code);
    return 0;
}

int response_on_header_field(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    if (c->was_reading_header_value)
    {
        if (!c->tmp_header_field.empty())
        {
            c->res->headers[c->tmp_header_field] = c->tmp_header_value;
            c->tmp_header_value.clear();
        }

        c->tmp_header_field = std::string(data, length);
        c->was_reading_header_value = false;
    }
    else
    {
        c->tmp_header_field += std::string(data, length);
    }
    return 0;
}

int response_on_header_value(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    if (!c->was_reading_header_value)
    {
        c->tmp_header_value = std::string(data, length);
        c->was_reading_header_value = true;
    }
    else
    {
        c->tmp_header_value += std::string(data, length);
    }
    return 0;
}

int response_on_headers_complete(http_parser *parser)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    if (!c->tmp_header_field.empty())
        c->res->headers[c->tmp_header_field] = c->tmp_header_value;
    return 0;
}

int response_on_body(http_parser *parser, const char *data, size_t length)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->res->body.reserve(c->res->body.size() + length);
    c->res->body.insert(c->res->body.end(), data, data + length);
    return 0;
}

int response_on_message_complete(http_parser *parser)
{
    auto c = reinterpret_cast<HTTPParser*>(parser->data);
    c->done = true;
    return 0;
}
