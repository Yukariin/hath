#include "HTTPClient.h"

#include "HTTPRequest.h"
#include "HTTPParser.h"

HTTPClient::HTTPClient() : service(), resolver(service), socket(service)
{

}

void HTTPClient::get(std::string url)
{
    std::shared_ptr<HTTPRequest> r = std::make_shared<HTTPRequest>("GET", url);
    r->header("Connection", "close");

    tcp::resolver::query q(r->url.host, r->url.protocol);
    tcp::resolver::iterator it = resolver.resolve(q);

    connect(socket, it);
    std::cout << r->url.pathAndQuery() << std::endl;

    write(socket, buffer(r->toHTTP()));

    HTTPParser p;
    std::array<char, 8192> buf;

    while (read(socket, buffer(buf)))
    {
        p.parseResponse(buf.data(), buf.size());
    }

    auto res = p.getResponse();
    std::cout << res->code << std::endl;
}
