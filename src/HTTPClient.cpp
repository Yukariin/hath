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

    HTTPParser p;
    std::array<char, 8192> buf;
    asio::error_code err;
    do
    {
        read(socket, buffer(buf), err);
        p.parseResponse(buf.data(), buf.size());
    } while (err != asio::error::eof);
    
    auto res = p.getResponse();
    
    Out::info(r->url.pathAndQuery() + ": Retrieving " + std::to_string(res->body.size()) + " bytes...");

    return res;
}
