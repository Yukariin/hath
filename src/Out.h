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

    static void disableLogging();
    static void flushLogs();

    static void debug(const std::string &s);
    static void info(const std::string &s);
    static void warning(const std::string &s);
    static void error(const std::string &s);

private:
    static bool writeLogs;
    static std::ofstream logout, logerr;
    static int logout_count, logerr_count;

    static std::ofstream startLogger(const std::string &logfile);
    static bool stopLogger(std::ofstream &logfile);

    static void log(const std::string &data, int severity);
    static void log(const std::string &data, std::ofstream &writer, bool flush);
    static void print(const std::string &s, const std::string &prefix, int severity);
};

#endif //HATH_OUT_H
