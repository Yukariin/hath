#include "HTTPClient.h"

#include "HTTPRequest.h"
#include "HTTPParser.h"
#include "Out.h"

HTTPClient::HTTPClient() : service(), resolver(service), socket(service)
{

}

std::shared_ptr<HTTPResponse> HTTPClient::get(std::string url)
{
    std::shared_ptr<HTTPRequest> r = std::make_shared<HTTPRequest>("GET", url);
    r->header("Connection", "close");

    tcp::resolver::query q(r->url.host, r->url.protocol);
    tcp::resolver::iterator it = resolver.resolve(q);

    connect(socket, it);

    write(socket, buffer(r->toHTTP()));

    HTTPParser p(HTTPParser::Response);
    std::array<char, 8192> buf;
    asio::error_code err;
    do
    {
        size_t readed = read(socket, buffer(buf), err);
        p.parseChunk(buf.data(), readed);
    } while (err != asio::error::eof);
    
    Out::info(r->url.pathAndQuery() + ": Retrieving " + std::to_string(p.res->body.size()) + " bytes...");

    return p.res;
}
