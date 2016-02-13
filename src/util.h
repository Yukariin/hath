#ifndef HATH_UTIL_H
#define HATH_UTIL_H

#ifdef _WIN32
    #include <string.h>
	#define strcasecmp _stricmp
#else
    #include <string.h>
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <stdint.h>
#include <iomanip>
#include <time.h>
#include <vector>
#include <random>
#include <functional>
#include <fstream>
#include <algorithm>
#include <map>
#include <chrono>

#include "boost/filesystem.hpp"
#include "Out.h"

using namespace boost::filesystem;

// Case-insensitive comparator.
struct ci_less : std::binary_function<std::string, std::string, bool>
{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

// Returns the current time as an HTTP-formatted date string.
inline std::string http_time(std::time_t now = std::time(nullptr))
{
    char buf[sizeof "Sun, 06 Nov 1994 08:49:37 GMT"];
    std::tm tm = *std::gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return std::string(buf);
}

inline std::string out_time(std::time_t now = std::time(nullptr))
{
    char buf[sizeof "2011-10-08T07:07:09Z"];
    std::tm tm = *std::gmtime(&now);
    strftime(buf, sizeof buf, "%FT%TZ", &tm);
    return std::string(buf);
}

inline long currentTime()
{
    auto epoch = std::chrono::system_clock::now().time_since_epoch();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(epoch);
    return duration.count();
}

/**
 * Splits a string by a delimiter.
 *
 * Based on an answer to http://stackoverflow.com/questions/236129
 */
inline std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems, int count = -1)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim) && elems.size() < count)
        elems.push_back(item);

    return elems;
}
/// @overload
inline std::vector<std::string> split(const std::string &s, char delim, int count = -1)
{
    std::vector<std::string> elems;
    split(s, delim, elems, count);
    return elems;
}

// http://stackoverflow.com/questions/3418231
inline std::string replace(std::string str, const std::string& from, const std::string& to, int count = -1)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos && count < 0 ? 1 : count-- > 0)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}

inline path checkAndCreateDir(std::string dir)
{
    path p(dir);
    if (is_regular_file(p))
        remove(p);

    if (!is_directory(p))
        create_directories(p);

    return p;
}

inline void putStringFileContents(std::string file, std::string content)
{
    std::ofstream out(file);
    out << content;
    out.close();
}

inline std::vector<char> get_rand_bytes(int size)
{
    std::vector<char> ret(size);
    std::fill(ret.begin(), ret.end(), (97 + rand() % 26));

    return ret;
}

std::map<std::string, std::string> parseAdditional(std::string additional);

#endif //HATH_UTIL_H
