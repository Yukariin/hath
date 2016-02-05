#include <bitset>
#include <sstream>

#include "URL.h"
#include "http_parser.h"

URL::URL(std::string str)
{
    const char *cstr = str.c_str();
    http_parser_url data = {0};
    if (http_parser_parse_url(cstr, str.size(), 0, &data) == 0)
    {
        port = data.port;

        std::bitset<16> set_fields(data.field_set);
        if (set_fields[UF_SCHEMA])
            protocol = std::string(cstr + data.field_data[UF_SCHEMA].off, data.field_data[UF_SCHEMA].len);
        if (set_fields[UF_HOST])
            host = std::string(cstr + data.field_data[UF_HOST].off, data.field_data[UF_HOST].len);
        if (set_fields[UF_PATH])
            path = std::string(cstr + data.field_data[UF_PATH].off, data.field_data[UF_PATH].len);
        if (set_fields[UF_QUERY])
            query = std::string(cstr + data.field_data[UF_QUERY].off, data.field_data[UF_QUERY].len);
        if (set_fields[UF_FRAGMENT])
            fragment = std::string(cstr + data.field_data[UF_FRAGMENT].off, data.field_data[UF_FRAGMENT].len);
    }
    else
        path = str;
}

std::string URL::str() const
{
    std::ostringstream ss;

    if(!protocol.empty()) ss << protocol << "://";
    else if(!host.empty()) ss << "http://";

    if(!host.empty()) ss << host;
    if(!path.empty()) ss << path;
    if(!query.empty()) ss << "?" << query;
    if(!fragment.empty()) ss << "#" << fragment;

    return ss.str();
}

std::string URL::pathAndQuery() const
{
    std::ostringstream ss;

    if (!path.empty()) ss << path;
    if (!query.empty()) ss << "?" << query;

    return ss.str();
}
