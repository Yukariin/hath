#include <iostream>
#include "HTTPConnectionManager.h"
#include "HTTPResponse.h"
#include "HTTPHandler.h"

HTTPConnection::HTTPConnection(io_service &service, HTTPConnectionManager &manager, HTTPHandler &handler)
        : socket(service), manager(manager), handler(handler), parser(HTTPParser::Request)
{

}

void HTTPConnection::start()
{
    do_read();
}

void HTTPConnection::do_read()
{
    auto self(shared_from_this());
    socket.async_read_some(asio::buffer(buffer),
    [this, self](const asio::error_code &error, std::size_t bytesTransferred)
    {
        if (!error)
        {
            HTTPParser::Status s = parser.parseChunk(buffer.data(), bytesTransferred);

            auto res = std::make_shared<HTTPResponse>();

            if (s == HTTPParser::GotRequest)
            {
                handler.handleRequest(parser.req, res);
                do_write(res->toHTTP());
            }
            else if (s == HTTPParser::Error)
            {
                res->begin(400)
                        ->end();
                do_write(res->toHTTP());
            }
            else // Keep going
            {
                do_read();
            }
        }
        else
            manager.stop(shared_from_this());
    });
}

void HTTPConnection::do_write(std::vector<char> data)
{
    long startTime = currentTimeMills();
    auto self(shared_from_this());
    async_write(socket, asio::buffer(data),
    [this, self, startTime](const asio::error_code& error, std::size_t bytesTransferred)
    {
        if (error) std::cout << error.message() << std::endl;

        long sendTime = currentTimeMills() - startTime;
        Out::info("Finished processing request in " + std::to_string(sendTime / 1000.0) + " seconds " + std::to_string(bytesTransferred / static_cast<float>(sendTime)));

        manager.stop(shared_from_this());
    });
}

void HTTPConnection::stop()
{
    if (socket.is_open())
    {
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
    }
}

tcp::socket& HTTPConnection::getSocket()
{
    return socket;
}
