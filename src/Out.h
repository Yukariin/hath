#ifndef HATH_OUT_H
#define HATH_OUT_H

#include <fstream>

class Out
{
public:
    static const int DEBUG = 1;
    static const int INFO = 2;
    static const int WARNING = 4;
    static const int ERROR = 8;

    static const int LOGOUT = DEBUG | INFO | WARNING | ERROR;
    static const int LOGERR = WARNING | ERROR;
    static const int OUTPUT = INFO | WARNING | ERROR;
    static const int VERBOSE = ERROR;

    Out();
    ~Out();

    static void debug(std::string x);
    static void info(std::string x);
    static void warning(std::string x);
    static void error(std::string x);

private:
    static std::ofstream logout, logerr;
    static int logout_count, logerr_count;

    static std::ofstream startLogger(std::string logfile);
    static bool stopLogger(std::ofstream &logfile);

    static void log(std::string data, int severity);
    static void log(std::string data, std::ofstream &writer, bool flush);
    static void print(std::string x, std::string name, int severity);
};

#endif //HATH_OUT_H
