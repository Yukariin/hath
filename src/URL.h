#ifndef HATH_URL_H
#define HATH_URL_H

#include <string>

class URL
{
public:
    URL(std::string str);

    uint16_t port;
    std::string protocol;
    std::string host;
    std::string path;
    std::string query;
    std::string fragment;

    std::string str() const;
    std::string pathAndQuery() const;
};

#endif //HATH_URL_H
