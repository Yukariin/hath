#include <iostream>
#include "HTTPConnectionManager.h"
#include "HTTPResponse.h"
#include "HTTPHandler.h"

HTTPConnection::HTTPConnection(tcp::socket socket, HTTPConnectionManager &manager, HTTPHandler &handler)
        : socket(std::move(socket)), manager(manager), handler(handler)
{

}

HTTPConnection::~HTTPConnection()
{

}

void HTTPConnection::read()
{
    auto self(shared_from_this());
    socket.async_read_some(asio::buffer(buffer),
    [this, self](const asio::error_code &error, std::size_t bytesTransferred)
    {
        if (!error)
        {
            HTTPParser::Status s = parser.parse(buffer.data(), bytesTransferred);
            auto req = parser.getRequest();
            auto res = std::make_shared<HTTPResponse>();

            if (s == HTTPParser::GotRequest)
            {
                handler.handleRequest(req, res);
                write(res->toHTTP());
            }
            else if (s == HTTPParser::Error)
            {
                res->begin(400)
                        ->end();
                write(res->toHTTP());
            }
            else // Keep going
            {
                read();
            }
        }
        else
            manager.stop(shared_from_this());
    });
}

void HTTPConnection::write(std::vector<char> data)
{
    auto self(shared_from_this());
    asio::async_write(socket, asio::buffer(data),
    [this, self](const asio::error_code& error, std::size_t bytesTransferred)
    {
        manager.stop(shared_from_this());
    });
}

void HTTPConnection::start()
{
    this->read();
}

void HTTPConnection::stop()
{
    if (socket.is_open())
    {
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
    }
}
