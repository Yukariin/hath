#include "Cache.h"

#include <mutex>
#include <algorithm>

#include "util.h"
#include "Settings.h"
#include "Out.h"
#include "HathClient.h"

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

    Out::info("Cache: Initializing database engine...");

    if (!initializeDatabase())
    {
        Out::info("");
        Out::info("**************************************************************************************************************");
        Out::info("The database could not be loaded. Please check file permissions and file system integrity.");
        Out::info("If everything appears to be working, please do the following:");
        Out::info("1. Locate the directory ");
        Out::info("2. Delete the file hath.db");
        Out::info("3. Restart the client.");
        Out::info("The system should now rebuild the database.");
        Out::info("***************************************************************************************************************");
        Out::info("");

        HathClient::dieWithError("Failed to load the database.");
    }

    if (quickStart)
        Out::info("Last shutdown was clean - using fast startup procedure.");
    else
        Out::info("Last shutdown was dirty - the cache index must be verified.");
    
    Out::info("Cache: Database initialized");
}

Cache::~Cache()
{
    terminateDatabase();
}

bool Cache::initializeDatabase()
{
    try
    {
        sqlite.exec("CREATE TABLE IF NOT EXISTS CacheList (fileid VARCHAR(65)  NOT NULL, lasthit INT UNSIGNED NOT NULL, filesize INT UNSIGNED NOT NULL, active BOOLEAN NOT NULL, PRIMARY KEY(fileid));");
        sqlite.exec("CREATE INDEX IF NOT EXISTS Lasthit ON CacheList (lasthit DESC);");
        sqlite.exec("CREATE TABLE IF NOT EXISTS StringVars ( k VARCHAR(255) NOT NULL, v VARCHAR(255) NOT NULL, PRIMARY KEY(k) );");

        // convert and clear pre-r81 tablespace if present.
        if (sqlite.tableExists("CacheIndex"))
        {
            sqlite.exec("UPDATE CacheIndex SET active=0;");

            Out::info("Updating database schema to r81...");
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
                insertCachedFile.bind(3, File::getHVFileFromFileid(i.first)->getSize());
                insertCachedFile.exec();
                insertCachedFile.reset();
            }

            t.commit();

            sqlite.exec("DROP TABLE CacheIndex;");
            Out::info("Database updates complete");
        }

        resetFutureLasthits();

        Out::info("CacheHandler: Optimizing database...");
        sqlite.exec("VACUUM;");
        
        if (!Settings::isForceDirty())
        {
            getStringVar.bind(1, CLEAN_SHUTDOWN_KEY);
            if (getStringVar.executeStep())
            {
                std::string val = getStringVar.getColumn(0);
                quickStart = val == CLEAN_SHUTDOWN_VALUE;
            }
            getStringVar.reset();
        }

        setStringVar.bind(1, CLEAN_SHUTDOWN_KEY);
        setStringVar.bind(2, std::to_string(currentTime()));
        setStringVar.exec();
        setStringVar.reset();

        return true;
    }
    catch (std::exception)
    {
        Out::error("CacheHandler: Encountered error reading database.");
        terminateDatabase();
    }

    return false;
}

void Cache::resetFutureLasthits()
{
    long nowtime = currentTime();

    Out::info("Cache: Checking future lasthits on non-static files...");

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
        deleteCachedFile.reset();
        boost::filesystem::remove(File::getHVFileFromFileid(fileid)->getLocalFilePath());
        Out::debug("Removed old static range file " + fileid);
    }

    Out::info("Cache: Resetting remaining far-future lasthits...");

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

void Cache::initializeCache()
{
    Out::info("Cache: Initializing the cache system...");

    for (auto &tmpfile : boost::filesystem::directory_iterator(tmpdir))
    {
        if (boost::filesystem::is_regular_file(tmpfile.path())) {
            Out::debug("Deleted orphaned temporary file " + tmpfile.path().filename().string());
            boost::filesystem::remove(tmpfile.path());
        }
        else
            Out::warning("Found a non-file " + tmpfile.path().filename().string() + " in the temp directory, won't delete.");
    }

    if (quickStart && !Settings::isVerifyCache())
    {
        try
        {
            if (cacheIndexCountStats.executeStep())
            {
                cacheCount = cacheIndexCountStats.getColumn(0);
                cacheSize = cacheIndexCountStats.getColumn(1);
            }
            cacheIndexCountStats.reset();
        }
        catch (std::exception &e)
        {
            Out::error("CacheHandler: Failed to perform database operation");
            HathClient::dieWithError(e);
        }
    }
    else
    {
        if (Settings::isVerifyCache())
            Out::info("CacheHandler: A full cache verification has been requested. This can take quite some time.");

        populateInternalCacheTable();
    }

    if (!Settings::isSkipFreeSpaceCheck() && (boost::filesystem::space(cachedir).free < Settings::getDiskLimitBytes() - cacheSize))
        HathClient::dieWithError("The storage device does not have enough space available to hold the given cache size.\nFree up space, or reduce the cache size from the H@H settings page.\nhttp://g.e-hentai.org/hentaiathome.php?cid=" + std::to_string(Settings::getClientID()));
    
    if ((cacheCount < 1) && (Settings::getStaticRangeCount() > 20))
        HathClient::dieWithError("This client has static ranges assigned to it, but the cache is empty.\nCheck permissions and, if necessary, delete the file hath.db in the data directory to rebuild the cache database.\nIf the cache has been deleted or is otherwise lost, you have to manually reset your static ranges from the H@H settings page.\nhttp://g.e-hentai.org/hentaiathome.php?cid=" + std::to_string(Settings::getClientID()));
    
    if (!checkAndFreeDiskSpace(cachedir.string(), true))
        Out::warning("ClientHandler: There is not enough space left on the disk to add more files to the cache.");
}

void Cache::addFileToActiveCache(std::shared_ptr<File> file)
{
    try
    {
        static std::mutex m;
        std::lock_guard<std::mutex> lock(m);

        std::string fileid = file->getFileid();

        updateCachedFileActive.bind(1, fileid);
        int affected = updateCachedFileActive.exec();
        updateCachedFileActive.reset();

        if (affected == 0)
        {
            long lasthit = currentTime();

            if (Settings::isStaticRange(fileid))
                // if the file is in a static range, bump to three months in the future. on the next access, it will get bumped further to a year.
                lasthit += 7776000;

            insertCachedFile.bind(1, fileid);
            insertCachedFile.bind(2, sqlite3_int64(lasthit));
            insertCachedFile.bind(3, file->getSize());
            insertCachedFile.exec();
            insertCachedFile.reset();
        }
    }
    catch (std::exception &e)
    {
        Out::error("CacheHandler: Failed to perform database operation");
        HathClient::dieWithError(e);
    }

    cacheCount++;
    cacheSize += file->getSize();
}

void Cache::pruneOldFiles()
{
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);

    std::vector<std::shared_ptr<File>> deleteNotify;
    int pruneCount = 0;

    Out::info("Checking for old files to prune...");

    try
    {
        queryCachedFileSortOnLasthit.bind(1, 0);
        queryCachedFileSortOnLasthit.bind(1, 20);
        long nowtime = currentTime();

        while (queryCachedFileSortOnLasthit.executeStep())
        {
            std::string fileid = queryCachedFileSortOnLasthit.getColumn(0);
            int lasthit = queryCachedFileSortOnLasthit.getColumn(1);

            if (lasthit < nowtime - 2592000)
            {
                auto toRemove = File::getHVFileFromFileid(fileid);
                if (toRemove != nullptr)
                {
                    pruneCount++;
                    deleteFileFromCacheNosync(toRemove);

                    if (!Settings::isStaticRange(fileid))
                        deleteNotify.push_back(toRemove);
                }
            }
        }
        queryCachedFileSortOnLasthit.reset();
    }
    catch (std::exception &e)
    {
        Out::error("CacheHandler: Failed to perform database operation");
        HathClient::dieWithError(e);
    }

    HathClient::instanse().getServerAPI()->notifyUncachedFiles(deleteNotify);
    
    Out::info("Pruned " + std::to_string(pruneCount) + " files.");
}

void Cache::processBlacklist(long deltatime, bool noServerDeleteNotify)
{
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);

    Out::info("CacheHandler: Retrieving list of blacklisted files...");
    auto blacklisted = HathClient::instanse().getServerAPI()->getBlacklist(deltatime);

    if (!blacklisted.size())
    {
        Out::warning("CacheHandler: Failed to retrieve file blacklist, will try again later.");
        return;
    }

    Out::info("CacheHandler: Looking for and deleting blacklisted files...");

    int counter = 0;
    std::vector<std::shared_ptr<File>> deleteNotify;

    try
    {
        for (std::string &fileid : blacklisted)
        {
            queryCachedFileLasthit.bind(1, fileid);
            std::shared_ptr<File> toRemove = nullptr;
            if (queryCachedFileLasthit.executeStep())
                toRemove = File::getHVFileFromFileid(fileid);
            queryCachedFileLasthit.reset();

            if (toRemove != nullptr)
            {
                counter++;
                deleteFileFromCacheNosync(toRemove);

                if (!Settings::isStaticRange(fileid))
                    deleteNotify.push_back(toRemove);
            }
        }
    }
    catch (std::exception &e)
    {
        Out::error("CacheHandler: Failed to perform database operation");
        HathClient::dieWithError(e);
    }

    if (!noServerDeleteNotify)
        HathClient::instanse().getServerAPI()->notifyUncachedFiles(deleteNotify);

    Out::info("CacheHandler: " + std::to_string(counter) + " blacklisted files were removed.");
}

void Cache::populateInternalCacheTable()
{
    try
    {
        cacheIndexClearActive.exec();
        cacheIndexClearActive.reset();

        cacheCount = 0;
        cacheSize = 0;

        int knownFiles = 0;
        int newFiles = 0;

        // load all the files directly from the cache directory itself and initialize the stored last access times for each file. last access times are used for the LRU-style cache.

        Out::info("CacheHandler: Loading cache.. (this could take a while)");

        try
        {
            int loadedFiles = 0;
            SQLite::Transaction t(sqlite);

            for (auto &scdir : boost::filesystem::directory_iterator(cachedir))
            {
                if (boost::filesystem::is_directory(scdir.path()))
                {
                    for (auto &cfile : boost::filesystem::directory_iterator(scdir.path()))
                    {
                        bool newFile = false;

                        {
                            static std::mutex m;
                            std::lock_guard<std::mutex> lock(m);

                            queryCachedFileLasthit.bind(1, cfile.path().filename().string());
                            newFile = !queryCachedFileLasthit.executeStep();
                            queryCachedFileLasthit.reset();
                        }

                        auto file = File::getHVFileFromFileid(cfile.path().filename().string());

                        if (file != nullptr)
                        {
                            addFileToActiveCache(file);

                            if (newFile)
                            {
                                newFiles++;
                                Out::info("CacheHandler: Verified and loaded file " + cfile.path().filename().string());
                            }
                            else
                            {
                                knownFiles++;
                            }

                            if(++loadedFiles % 1000 == 0) {
                                Out::info("CacheHandler: Loaded " + std::to_string(loadedFiles) + " files so far...");
                            }
                        }
                        else
                        {
                            Out::warning("CacheHandler: The file " + cfile.path().filename().string() + " was corrupt. It is now deleted.");
                            boost::filesystem::remove(cfile.path());
                        }
                    }
                }
                else
                {
                    boost::filesystem::remove(scdir.path());
                }
            }

            t.commit();

            {
                static std::mutex m;
                std::lock_guard<std::mutex> lock(m);

                int purged = deleteCachedFileInactive.exec();
                deleteCachedFileInactive.reset();
                Out::info("CacheHandler: Purged " + std::to_string(purged) + " nonexisting files from database.");
            }
        }
        catch (std::exception &e)
        {
            Out::error("CacheHandler: Failed to perform database operation");
            HathClient::dieWithError(e);
        }

        Out::info("CacheHandler: Loaded " + std::to_string(knownFiles) + " known files.");
        Out::info("CacheHandler: Loaded " + std::to_string(newFiles) + " new files.");
        Out::info("CacheHandler: Finished initializing the cache (" + std::to_string(cacheCount) + " files, " + std::to_string(cacheSize) + " bytes)");
    }
    catch (std::exception &e)
    {
        Out::error(e.what());
        HathClient::dieWithError("HathClient::dieWithError(e);");
    }
}

boost::filesystem::path Cache::getCacheDir()
{
    return cachedir;
}

boost::filesystem::path Cache::getTmpDir()
{
    return tmpdir;
}

int Cache::getCacheCount()
{
    return cacheCount;
}

void Cache::deleteFileFromCache(std::shared_ptr<File> toRemove)
{
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);

    deleteFileFromCacheNosync(toRemove);
}

void Cache::deleteFileFromCacheNosync(std::shared_ptr<File> toRemove)
{
    try
    {
        deleteCachedFile.bind(1, toRemove->getFileid());
        deleteCachedFile.exec();
        deleteCachedFile.reset();
        cacheCount--;
        cacheSize -= toRemove->getSize();
        boost::filesystem::remove(toRemove->getLocalFilePath());
        Out::info("CacheHandler: Deleted cached file " + toRemove->getFileid());
    }
    catch (std::exception &e)
    {
        Out::error("CacheHandler: Failed to perform database operation");
        HathClient::dieWithError(e);
    }
}

bool Cache::checkAndFreeDiskSpace(std::string file, bool noServerDeleteNotify)
{
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);

    if (!file.size())
        HathClient::dieWithError("CacheHandler: checkAndFreeDiskSpace needs a file handle to calculate free space");

    int bytesNeeded = boost::filesystem::is_directory(file) ? 0 : static_cast<int>(boost::filesystem::file_size(file));
    long cacheLimit = Settings::getDiskLimitBytes();
    
    Out::debug("CacheHandler: Checking disk space (adding " + std::to_string(bytesNeeded) + " bytes: cacheSize=" + std::to_string(cacheSize) + ", cacheLimit=" + std::to_string(cacheLimit) + ", cacheFree=" + std::to_string(cacheLimit - cacheSize) + ")");
    
    long bytesToFree = 0;
    if (cacheSize > cacheLimit)
        bytesToFree = cacheSize - cacheLimit;
    else if(cacheSize + bytesNeeded - cacheLimit > 0)
        bytesToFree = bytesNeeded * 10;
    
    if (bytesToFree > 0)
    {
        Out::info("CacheHandler: Freeing at least " + std::to_string(bytesToFree) + " bytes...");
        std::vector<std::shared_ptr<File>> deleteNotify;
        
        try
        {
            while (bytesToFree > 0 && cacheCount > 0)
            {
                {
                    static std::mutex m;
                    std::lock_guard<std::mutex> lock(m);
                    
                    queryCachedFileSortOnLasthit.bind(1, 0);
                    queryCachedFileSortOnLasthit.bind(2, 20);
                    
                    while (queryCachedFileSortOnLasthit.executeStep())
                    {
                        std::string fileid = queryCachedFileSortOnLasthit.getColumn(0);
                        auto toRemove = File::getHVFileFromFileid(fileid);
                        
                        if (toRemove != nullptr)
                        {
                            deleteFileFromCacheNosync(toRemove);
                            bytesToFree -= toRemove->getSize();
                            
                            if (!Settings::isStaticRange(fileid))
                                deleteNotify.push_back(toRemove);
                        }
                    }
                    queryCachedFileSortOnLasthit.reset();
                }
            }
        }
        catch (std::exception &e)
        {
            Out::error("CacheHandler: Failed to perform database operation");
            HathClient::dieWithError(e);
        }
        
        if (!noServerDeleteNotify)
            HathClient::instanse().getServerAPI()->notifyUncachedFiles(deleteNotify);
    }
    
    if (Settings::isSkipFreeSpaceCheck())
    {
        Out::debug("CacheHandler: Disk free space check is disabled.");
        return true;
    }
    else
    {
        long diskFreeSpace = boost::filesystem::space(file).free;
        if (diskFreeSpace < std::max(Settings::getDiskMinRemainingBytes(), (long)(104857600)))
        {
            Out::warning("CacheHandler: Cannot meet space constraints: Disk free space limit reached (" + std::to_string(diskFreeSpace) + " bytes free on device)");
            return false;
        }
        else
        {
            Out::debug("CacheHandler: Disk space constraints met (" + std::to_string(diskFreeSpace) + " bytes free on device)");
            return true;
        }
    }
}

int Cache::getSegmentCount()
{
    return (Settings::isUseLessMemory() && cacheCount > 16000) || (cacheCount > 400000) ? 256 : 16;
}

int Cache::getStartupCachedFilesStrlen()
{
    return startupCachedFileStrlen;
}

void Cache::calculateStartupCachedFilesStrlen()
{
    int segmentCount = getSegmentCount();
    startupCachedFileStrlen = 0;

    for (int segmentIndex = 0; segmentIndex < segmentCount; segmentIndex++)
    {
        auto fileList = getCachedFilesSegment(toHexString(segmentCount | segmentIndex).substr(1));
        
        for (std::string fileid : fileList)
            startupCachedFileStrlen += fileid.size() + 1;
        
        Out::info("Calculated segment " + std::to_string(segmentIndex) + " of " + std::to_string(segmentCount));
    }
}

std::vector<std::string> Cache::getCachedFilesSegment(std::string segment)
{
    std::vector<std::string> fileList;
    
    try
    {
        static std::mutex m;
        std::lock_guard<std::mutex> lock(m);
        
        queryCachelistSegment.bind(1, segment + "0");
        queryCachelistSegment.bind(2, segment + "g");
        while (queryCachelistSegment.executeStep())
        {
            std::string fileid = queryCachelistSegment.getColumn(0);
            if (!Settings::isStaticRange(fileid))
                fileList.push_back(fileid);
        }
        queryCachelistSegment.reset();
    }
    catch (std::exception &e)
    {
        Out::error("CacheHandler: Failed to perform database operation");
        HathClient::dieWithError(e);
    }
    
    return fileList;
}
