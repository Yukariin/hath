#include "HTTPServer.h"

HTTPServer::HTTPServer() : acceptor(service)
{

}

HTTPServer::~HTTPServer()
{
    acceptor.close();
    service.stop();
}

asio::error_code HTTPServer::listen(const tcp::endpoint &endpoint)
{
    asio::error_code error;

    acceptor.open(endpoint.protocol(), error);
    if (error) return error;

    acceptor.bind(endpoint, error);
    if (error) return error;

    acceptor.listen(socket_base::max_connections, error);
    if (error) return error;

    return error;
}

asio::error_code HTTPServer::listen(const std::string &address, const uint16_t &port)
{
    return this->listen(tcp::endpoint(address::from_string(address), port));
}

asio::error_code HTTPServer::listen(const uint16_t &port)
{
    return this->listen(tcp::endpoint(tcp::v4(), port));
}

void HTTPServer::run()
{
    service.run();
}

void HTTPServer::stop()
{
    service.stop();
}
