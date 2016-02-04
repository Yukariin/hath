#ifndef HATH_CACHE_H
#define HATH_CACHE_H

#include <string>
#include <boost/filesystem/path.hpp>

#include "SQLiteCpp/SQLiteCpp.h"
#include "File.h"

class Cache
{
public:
    Cache();
    ~Cache();

    void terminateDatabase();
    void initializeCacheHandler();
    void addFileToActiveCache(File);

    static boost::filesystem::path getCacheDir();

private:
    static boost::filesystem::path cachedir;
    static boost::filesystem::path tmpdir;

    int cacheCount;
    long cacheSize;

    SQLite::Database sqlite;
    SQLite::Statement cacheIndexClearActive, cacheIndexCountStats;
    SQLite::Statement queryCachelistSegment, queryCachedFileLasthit, queryCachedFileSortOnLasthit;
    SQLite::Statement insertCachedFile, updateCachedFileLasthit, updateCachedFileActive;
    SQLite::Statement deleteCachedFile, deleteCachedFileInactive;
    SQLite::Statement getStringVar, setStringVar;

    std::vector<File> pendingRegister;

    const std::string CLEAN_SHUTDOWN_KEY = "clean_shutdown";
    const std::string CLEAN_SHUTDOWN_VALUE = "clean_r81";

    bool initializeDatabase();
    void resetFutureLasthits();
    void populateInternalCacheTable();
};

#endif //HATH_CACHE_H
