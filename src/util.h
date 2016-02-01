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

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

// Case-insensitive comparator.
struct ci_less : std::binary_function<std::string, std::string, bool>
{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};


/* SHA-1 hash
 *
 * Copyright (c) 2014 Project Nayuki
 * http://www.nayuki.io/page/fast-sha1-hash-implementation-in-x86-assembly
 */
void sha1_compress(uint32_t state[5], const uint8_t block[64]);
void get_sha1_hash(const uint8_t *message, uint32_t len, uint32_t hash[5]);
std::string get_sha1_string(std::string s);

// Returns the current time as an HTTP-formatted date string.
inline std::string http_time(std::time_t now = std::time(nullptr))
{
    char buf[1000];
    std::tm tm = *std::gmtime(&now);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return std::string(buf);
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
    while (std::getline(ss, item, delim) && count < 0 ? 1 : count-- > 0)
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

inline std::string checkAndCreateDir(std::string dir)
{
    path p(dir);
    if (is_regular_file(p))
        remove(p);

    if (!is_directory(p))
        create_directories(p);

    return dir;
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

#endif //HATH_UTIL_H
