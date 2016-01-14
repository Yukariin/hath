#ifndef HATH_UTIL_H
#define HATH_UTIL_H

#ifdef _WIN32
    #include <string.h>
	#define strcasecmp _stricmp
#else
    #include <string.h>
#endif

#include <string>
#include <sstream>
#include <stdint.h>
#include <iomanip>

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

#endif //HATH_UTIL_H
