#include <mutex>
#include <iostream>
#include <sstream>
#include "Out.h"

#include "boost/filesystem.hpp"
#include "Settings.h"
#include "util.h"

using namespace boost::filesystem;

bool Out::writeLogs;
std::ofstream Out::logout;
std::ofstream Out::logerr;
int Out::logout_count;
int Out::logerr_count;

Out::Out()
{
    Settings::initializeDataDir();

    writeLogs = true;

    logout = startLogger("data/log_out");
    logerr = startLogger("data/log_err");
}

Out::~Out()
{
    stopLogger(logout);
    stopLogger(logerr);
}

void Out::disableLogging()
{
    if (writeLogs)
    {
        info("Logging ended.");
        writeLogs = false;
        flushLogs();
    }
}

void Out::flushLogs()
{
    logout.flush();
}

std::ofstream Out::startLogger(const std::string &logfile)
{
    if (logfile.size())
    {
        if (exists(logfile + ".old"))
            remove(path(logfile + ".old"));
        if (exists(logfile))
            rename(path(logfile), path(logfile + ".old"));

        return std::ofstream(logfile);
    }

    return std::ofstream();
}

bool Out::stopLogger(std::ofstream &logfile)
{
    logfile.close();

    return true;
}

void Out::debug(const std::string &s)
{
    print(s, "debug", DEBUG);
}

void Out::info(const std::string &s)
{
    print(s, "info", INFO);
}

void Out::warning(const std::string &s)
{
    print(s, "WARN", WARNING);
}

void Out::error(const std::string &s)
{
    print(s, "ERROR", ERROR);
}

void Out::log(const std::string &data, int severity)
{
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);

    if (((severity & LOGOUT) > 0) && writeLogs)
    {
        log(data, logout, false);
        if (++logout_count > 100000)
        {
            logout_count = 0;
            std::cout << "Rotating output logfile..." << std::endl;
            if (stopLogger(logout))
            {
                logout = startLogger("data/log_out");
                std::cout << "Output logfile rotated." << std::endl;
            }
        }
    }

    if ((severity & LOGERR) > 0)
    {
        log(data, logerr, true);
        if (++logerr_count > 100000)
        {
            logerr_count = 0;
            std::cout << "Rotating error logfile..." << std::endl;
            if (stopLogger(logerr))
            {
                logerr = startLogger("data/log_err");
                std::cout << "Error logfile rotated." << std::endl;
            }
        }
    }
}

void Out::log(const std::string &data, std::ofstream &writer, bool flush)
{
    if (writer.is_open())
    {
        writer << data << std::endl;

        if (flush)
            writer.flush();
    }
}

void Out::print(const std::string &s, const std::string &prefix, int severity)
{
    std::string nowtime = out_time();
    bool output = (severity & OUTPUT) > 0;
    bool log = (severity & (LOGOUT | LOGERR)) > 0;

    if (output || log)
    {
        static std::mutex m;
        std::lock_guard<std::mutex> lock(m);
        
        auto spl = split(s, '\n');
        for (std::string &str : spl)
        {
            std::ostringstream data;
            data << nowtime
            << " [" << prefix << "] "
            << str;

            if (output)
                std::cout << data.str() << std::endl;

            if (log)
                Out::log(data.str(), severity);
        }
    }
}
