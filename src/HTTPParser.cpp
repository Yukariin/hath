#include "HTTPParser.h"

HTTPParser::HTTPParser()
{
    http_parser_init(&parser, HTTP_REQUEST);
    parser.data = this;
}

HTTPParser::~HTTPParser()
{

}

HTTPParser::Status HTTPParser::parse(const char *data, std::size_t length)
{
    parser_settings.on_message_begin = [](http_parser *parser)
    {
        auto c = reinterpret_cast<HTTPParser*>(parser->data);
        c->req = std::make_shared<HTTPRequest>();
        return 0;
    };

    parser_settings.on_url = [](http_parser *parser, const char *dt, size_t l)
        {
            auto c = reinterpret_cast<HTTPParser*>(parser->data);
            c->req->url += std::string(dt, l);
            return 0;
        };

    parser_settings.on_header_field = [](http_parser *parser, const char *dt, size_t l)
    {
        auto c = reinterpret_cast<HTTPParser*>(parser->data);
        if (c->was_reading_header_value)
        {
            if (!c->tmp_header_field.empty())
            {
                c->req->headers[c->tmp_header_field] = c->tmp_header_value;
                c->tmp_header_value.clear();
            }

            c->tmp_header_field = std::string(dt, l);
            c->was_reading_header_value = false;
        }
        else
        {
            c->tmp_header_field += std::string(dt, l);
        }
        return 0;
    };

    parser_settings.on_header_value = [](http_parser *parser, const char *dt, size_t l)
    {
        auto c = reinterpret_cast<HTTPParser*>(parser->data);
        if (!c->was_reading_header_value)
        {
            c->tmp_header_value = std::string(dt, l);
            c->was_reading_header_value = true;
        }
        else
        {
            c->tmp_header_value += std::string(dt, l);
        }
        return 0;
    };

    parser_settings.on_headers_complete = [](http_parser *parser)
    {
        auto c = reinterpret_cast<HTTPParser*>(parser->data);
        if (!c->tmp_header_field.empty())
            c->req->headers[c->tmp_header_field] = c->tmp_header_value;
        return 0;
    };

    parser_settings.on_body = [](http_parser *parser, const char *dt, size_t l)
    {
        auto c = reinterpret_cast<HTTPParser*>(parser->data);
        c->req->body.reserve(c->req->body.size() + l);
        c->req->body.insert(c->req->body.end(), dt, dt + l);
        return 0;
    };

    parser_settings.on_message_complete = [](http_parser *parser)
    {
        auto c = reinterpret_cast<HTTPParser*>(parser->data);
        c->req->method = http_method_str(static_cast<http_method>(parser->method));
        c->done = true;
        return 0;
    };

    if (done)
    {
        done = false;
        req = nullptr;
    }

    std::size_t parsed = http_parser_execute(&parser, &parser_settings, data, length);

    if (parsed != length)
        return Error;

    return (done ? GotRequest : KeepGoing);
}

std::shared_ptr<HTTPRequest> HTTPParser::getRequest()
{
    return req;
}
