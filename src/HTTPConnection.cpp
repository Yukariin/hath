#include <iostream>
#include "HTTPConnection.h"

#include "HTTPConnectionManager.h"

HTTPConnection::HTTPConnection(tcp::socket socket, HTTPConnectionManager &manager)
        : socket(std::move(socket)), manager(manager)
{

}

void HTTPConnection::read()
{
    auto self(shared_from_this());
    socket.async_read_some(asio::buffer(buffer, BufferSize),
    [&](const asio::error_code &error, std::size_t bytesTransferred)
    {
        std::cout << bytesTransferred << std::endl;

        if (!error)
            read();
        else
            manager.stop(shared_from_this());
    });
}

void HTTPConnection::write()
{
    auto self(shared_from_this());
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
