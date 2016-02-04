#include <iostream>
#include <thread>

#include "HTTPServer.h"

HTTPServer::HTTPServer(unsigned int workers)
        : service(), work(service), acceptor(service), socket(service), manager(), request_handler()
{
    for(unsigned int i = 0; i < workers; i++)
        worker_threads.emplace_back([&]{ service.run(); });
}

HTTPServer::~HTTPServer()
{
    acceptor.close();
    manager.stopAll();
    service.stop();
    for(auto it = worker_threads.begin(); it != worker_threads.end(); it++)
        it->join();
}

asio::error_code HTTPServer::listen(const tcp::endpoint &endpoint)
{
    asio::error_code error;

    acceptor.open(endpoint.protocol(), error);
    if (error) return error;

    acceptor.set_option(tcp::acceptor::reuse_address(true), error);
    if (error) return error;

    acceptor.bind(endpoint, error);
    if (error) return error;

    acceptor.listen(socket_base::max_connections, error);
    if (error) return error;

    this->accept();
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

void HTTPServer::accept()
{
    acceptor.async_accept(socket, [=](const asio::error_code &error)
    {
        if (!acceptor.is_open())
            return;

        if (!error)
        {
            manager.start(std::make_shared<HTTPConnection>(std::move(socket), manager, request_handler));
        }

        this->accept();
    });
}
