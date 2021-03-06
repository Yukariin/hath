#include <iostream>
#include <thread>

#include "HTTPServer.h"
#include "Out.h"

HTTPServer::HTTPServer(unsigned int workers)
        : service(), work(service), acceptor(service), manager(), request_handler()
{
    for(unsigned int i = 0; i < workers; i++)
        worker_threads.emplace_back([&]{ service.run(); });
}

HTTPServer::~HTTPServer()
{
    acceptor.close();
    manager.stopAll();
    service.stop();
    for(auto &t : worker_threads)
        t.join();
}

asio::error_code HTTPServer::listen(const tcp::endpoint &endpoint)
{
    Out::info("Starting up the internal HTTP Server...");

    asio::error_code error;

    acceptor.open(endpoint.protocol(), error);
    if (error) return error;

    acceptor.set_option(tcp::acceptor::reuse_address(true), error);
    if (error) return error;

    acceptor.bind(endpoint, error);
    if (error) return error;

    acceptor.listen(socket_base::max_connections, error);
    if (error) return error;

    Out::info("Internal HTTP Server was successfully started, and is listening on port " + std::to_string(acceptor.local_endpoint().port()));

    accept();
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
    newConnection = std::make_shared<HTTPConnection>(service, manager, request_handler);
    
    acceptor.async_accept(newConnection->getSocket(), [=](const asio::error_code &error)
    {
        if (!acceptor.is_open())
            return;

        if (!error)
        {
            manager.start(newConnection);
        }

        accept();
    });
}
