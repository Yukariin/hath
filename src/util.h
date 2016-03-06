#ifndef HATH_UTIL_H
#define HATH_UTIL_H

#ifdef _WIN32
    #include <string.h>
	#define strcasecmp _stricmp
#else
    #include <string.h>
#endif

#include <time.h>

#include <iostream>
#include <string>
#include <sstream>
#include <stdint.h>
#include <iomanip>
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
inline std::string http_date(std::time_t now = std::time(nullptr))
{
    char buf[sizeof "Sun, 06 Nov 1994 08:49:37 GMT"]; // 29
    std::tm tm = *std::gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %T %Z", &tm);
    
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

inline long currentTimeMills()
{
    auto epoch = std::chrono::system_clock::now().time_since_epoch();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

    return duration.count();
}

// returns count of non-overlapping occurrences of 'sub' in 'str'
inline int countSubstring(const std::string& str, const std::string& sub)
{
    if (sub.length() == 0) return 0;
    
    int count = 0;
    for (size_t offset = str.find(sub); offset != std::string::npos;
         offset = str.find(sub, offset + sub.length()))
    {
        ++count;
    }
    
    return count;
}

/**
 * Splits a string by a delimiter.
 *
 * Based on Go genSplit
 */
inline std::vector<std::string> split(const std::string &s, char delim, int n = -1)
{
    if (n < 0)
        n = std::count(s.begin(), s.end(), delim) + 1;
        
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    
    while (std::getline(ss, item, delim) && elems.size() + 1 < n)
    {
        elems.push_back(item);

    }
    std::getline(ss, item);
    elems.push_back(item);
    
    return elems;
}

// http://stackoverflow.com/questions/3418231
inline std::string replace(std::string str, const std::string& from, const std::string& to, int n = -1)
{
    if (from == to || n == 0)
        return str;
    
    int m = countSubstring(str, from);
    if (m == 0)
        return str;
    else if (n < 0 || m < n)
        n = m;
    
    size_t start_pos = 0;
    for (int i = 0; i < n; i++)
    {
        start_pos = str.find(from, start_pos);
        if (start_pos != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        else
            return str;
    }

    return str;
}

inline std::string toHexString(int i)
{
    std::stringstream ss;
    ss << std::hex << i;
    return ss.str();
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
    auto randchar = []() -> char
    {
        return 'a' + rand() % 26;
    };
    
    std::vector<char> ret(size);
    std::generate(ret.begin(), ret.end(), randchar);

    return ret;
}

std::map<std::string, std::string> parseAdditional(std::string additional);

#endif //HATH_UTIL_H
