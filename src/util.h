#ifndef HATH_UTIL_H
#define HATH_UTIL_H

#ifdef _WIN32
    #include <string.h>
	#define strcasecmp _stricmp
#else
    #include <string.h>
#endif

#include <string>

// Case-insensitive comparator.
struct ci_less : std::binary_function<std::string, std::string, bool>
{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

#endif //HATH_UTIL_H
