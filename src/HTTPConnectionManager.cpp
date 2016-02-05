#include "HTTPConnectionManager.h"

void HTTPConnectionManager::start(connectionPtr c)
{
    connections.insert(c);
    c->start();
}

void HTTPConnectionManager::stop(connectionPtr c)
{
    connections.erase(c);
    c->stop();
}

void HTTPConnectionManager::stopAll()
{
    for (auto c: connections)
        c->stop();
    connections.clear();
}
