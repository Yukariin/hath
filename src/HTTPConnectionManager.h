#ifndef HATH_HTTPCONNECTIONMANAGER_H
#define HATH_HTTPCONNECTIONMANAGER_H

#include <set>

#include "HTTPConnection.h"

class HTTPConnectionManager
{
public:
    void start(connectionPtr c);
    void stop(connectionPtr c);
    void stopAll();

private:
    std::set<connectionPtr> connections;
};


#endif //HATH_HTTPCONNECTIONMANAGER_H
