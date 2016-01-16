#include <iostream>
#include "HTTPHandler.h"

HTTPHandler::HTTPHandler()
{

}

void HTTPHandler::handleRequest(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res)
{
    if (req->method != "GET" && req->method != "HEAD")
        res->begin(405)
                ->end();
    else {
        std::cout << req->url << std::endl;
        auto urlparts = split(req->url, '/');
        std::cout << urlparts.size() << std::endl;
        if (urlparts.size() < 1)
        {
            res->begin(404)
                    ->end();
        }
        else
        {
            if (urlparts[0] == "servercmd")
            {
                // form: /servercmd/$command/$additional/$time/$key
            }
            else if (urlparts[0] == "favicon.ico")
            {
                res->begin(301)
                        ->header("Location", "http://g.e-hentai.org/favicon.ico")
                        ->end();
            }
            else if (urlparts[0] == "robots.txt")
            {
                res->begin()
                        ->write("User-agent: *\nDisallow: /")
                        ->end();
            }
            else
            {
                res->begin(404)
                        ->end();
            }
        }
    }
}
