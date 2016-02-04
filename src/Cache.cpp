#include "Cache.h"

#include "util.h"
#include "Settings.h"

boost::filesystem::path Cache::cachedir;
boost::filesystem::path Cache::tmpdir;

Cache::Cache() :
        sqlite("data/hath.db", SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE),
        cacheIndexClearActive(sqlite, "UPDATE CacheList SET active=0;"),
        cacheIndexCountStats(sqlite, "SELECT COUNT(), SUM(filesize) FROM CacheList;"),
        queryCachelistSegment(sqlite, "SELECT fileid FROM CacheList WHERE fileid BETWEEN ? AND ?;"),
        queryCachedFileLasthit(sqlite, "SELECT lasthit FROM CacheList WHERE fileid=?;"),
        queryCachedFileSortOnLasthit(sqlite, "SELECT fileid, lasthit, filesize FROM CacheList ORDER BY lasthit LIMIT ?, ?;"),
        insertCachedFile(sqlite, "INSERT OR REPLACE INTO CacheList (fileid, lasthit, filesize, active) VALUES (?, ?, ?, 1);"),
        updateCachedFileLasthit(sqlite, "UPDATE CacheList SET lasthit=? WHERE fileid=?;"),
        updateCachedFileActive(sqlite, "UPDATE CacheList SET active=1 WHERE fileid=?;"),
        deleteCachedFile(sqlite, "DELETE FROM CacheList WHERE fileid=?;"),
        deleteCachedFileInactive(sqlite, "DELETE FROM CacheList WHERE active=0;"),
        getStringVar(sqlite, "SELECT v FROM StringVars WHERE k=?;"),
        setStringVar(sqlite, "INSERT OR REPLACE INTO StringVars (k, v) VALUES (?, ?);")
{
    cachedir = checkAndCreateDir("cache");
    tmpdir = checkAndCreateDir("tmp");

    initializeDatabase();
}

Cache::~Cache()
{
    terminateDatabase();
}

bool Cache::initializeDatabase()
{
    sqlite.exec("CREATE TABLE IF NOT EXISTS CacheList (fileid VARCHAR(65)  NOT NULL, lasthit INT UNSIGNED NOT NULL, filesize INT UNSIGNED NOT NULL, active BOOLEAN NOT NULL, PRIMARY KEY(fileid));");
    sqlite.exec("CREATE INDEX IF NOT EXISTS Lasthit ON CacheList (lasthit DESC);");
    sqlite.exec("CREATE TABLE IF NOT EXISTS StringVars ( k VARCHAR(255) NOT NULL, v VARCHAR(255) NOT NULL, PRIMARY KEY(k) );");

    // convert and clear pre-r81 tablespace if present.
    if (sqlite.tableExists("CacheIndex"))
    {
        sqlite.exec("UPDATE CacheIndex SET active=0;");

        std::map<std::string, int> hashtable;
        SQLite::Statement s(sqlite, "SELECT fileid, lasthit FROM CacheIndex;");
        while (s.executeStep())
        {
            std::string fileid = s.getColumn(0);
            int lasthit = s.getColumn(1);
            hashtable.insert(std::make_pair(fileid, lasthit));
        }

        SQLite::Transaction t(sqlite);

        for (auto i : hashtable)
        {
            insertCachedFile.bind(1, i.first);
            insertCachedFile.bind(2, i.second);
            insertCachedFile.bind(3, File::getHVFileFromFileid(i.first).getSize());
            insertCachedFile.exec();
        }

        t.commit();

        sqlite.exec("DROP TABLE CacheIndex;");
    }

    resetFutureLasthits();

    sqlite.exec("VACUUM;");

    setStringVar.bind(1, CLEAN_SHUTDOWN_KEY);
    setStringVar.bind(2, std::to_string(currentTime()));
    setStringVar.exec();

    return true;
}

void Cache::resetFutureLasthits()
{
    long nowtime = currentTime();

    std::vector<std::string> removelist;
    SQLite::Statement s(sqlite, "SELECT fileid FROM CacheList WHERE lasthit>?;");
    s.bind(1, sqlite3_int64(nowtime + 2592000));
    while (s.executeStep())
    {
        std::string fileid = s.getColumn(0);
        if (!Settings::isStaticRange(fileid))
            removelist.push_back(fileid);
    }

    SQLite::Transaction t(sqlite);

    for (std::string fileid : removelist)
    {
        deleteCachedFile.bind(1, fileid);
        deleteCachedFile.exec();
        boost::filesystem::remove(File::getHVFileFromFileid(fileid).getLocalFilePath());
    }

    SQLite::Statement u(sqlite, "UPDATE CacheList SET lasthit=? WHERE lasthit>?;");
    u.bind(1, sqlite3_int64(nowtime + 7776000));
    u.bind(2, sqlite3_int64(nowtime + 31536000));
    u.exec();

    t.commit();
}

void Cache::terminateDatabase()
{
    try
    {
        setStringVar.bind(1, CLEAN_SHUTDOWN_KEY);
        setStringVar.bind(2, CLEAN_SHUTDOWN_VALUE);
        setStringVar.exec();
    }
    catch (std::exception &e) {}

}

void Cache::initializeCacheHandler()
{
    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator dir_iter(tmpdir); dir_iter != end_iter ; ++dir_iter)
    {
        if (boost::filesystem::is_regular_file(dir_iter->path()))
            boost::filesystem::remove(dir_iter->path());
        else
            std::cout << "Found a non-file " << dir_iter->path() << " in the temp directory, won't delete." << std::endl;
    }

    populateInternalCacheTable();
}

void Cache::addFileToActiveCache(File file)
{
    std::string fileid = file.getFileid();

    updateCachedFileActive.bind(1, fileid);
    int affected = updateCachedFileActive.exec();

    if (affected == 0)
    {
        long lasthit = currentTime();

        if (Settings::isStaticRange(fileid))
            // if the file is in a static range, bump to three months in the future. on the next access, it will get bumped further to a year.
            lasthit += 7776000;

        insertCachedFile.bind(1, fileid);
        insertCachedFile.bind(2, sqlite3_int64(lasthit));
        insertCachedFile.bind(3, file.getSize());
        insertCachedFile.exec();
    }

    cacheCount++;
    cacheSize += file.getSize();
}

void Cache::populateInternalCacheTable()
{
    cacheIndexClearActive.exec();

    cacheCount = 0;
    cacheSize = 0;

    // load all the files directly from the cache directory itself and initialize the stored last access times for each file. last access times are used for the LRU-style cache.

    std::cout << "Cache: Loading cache.. (this could take a while)" << std::endl;

    SQLite::Transaction t(sqlite);
    t.commit();
}

boost::filesystem::path Cache::getCacheDir()
{
    return cachedir;
}
