#ifndef HATH_CACHE_H
#define HATH_CACHE_H

#include <string>

#include "boost/filesystem/path.hpp"
#include "SQLiteCpp/SQLiteCpp.h"

#include "File.h"

class Cache
{
public:
    Cache();
    ~Cache();

    void terminateDatabase();
    void initializeCache();
    void addFileToActiveCache(std::shared_ptr<File> file);
    void pruneOldFiles();
    void processBlacklist(long deltatime, bool noServerDeleteNotify);
    void deleteFileFromCache(std::shared_ptr<File> toRemove);
    int getSegmentCount();
    int getStartupCachedFilesStrlen();
    void calculateStartupCachedFilesStrlen();
    std::vector<std::string> getCachedFilesSegment(std::string segment);

    static boost::filesystem::path getCacheDir();
    static boost::filesystem::path getTmpDir();

    int getCacheCount();

private:
    static boost::filesystem::path cachedir;
    static boost::filesystem::path tmpdir;

    bool quickStart = false;

    int cacheCount, startupCachedFileStrlen;
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
    void deleteFileFromCacheNosync(std::shared_ptr<File> toRemove);
    bool checkAndFreeDiskSpace(std::string file, bool noServerDeleteNotify);
};

#endif //HATH_CACHE_H
