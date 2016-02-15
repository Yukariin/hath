#ifndef HATH_SETTINGS_H
#define HATH_SETTINGS_H

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <map>

class Settings
{
public:
    static const int CLIENT_BUILD = 98;
    static const std::string CLIENT_VERSION;

    static const std::string CLIENT_API_URL;

    static const std::string DATA_FILENAME_CLIENT_LOGIN;
    static const std::string DATA_FILENAME_LASTHIT_HISTORY;

    static const int CLIENT_KEY_LENGTH = 20;
    static const int MAX_KEY_TIME_DRIFT = 300;
    static const int MAX_CONNECTION_BASE = 20;

    static bool loginCredentialsAreSyntaxValid();
    static bool loadClientLoginFromFile();
    static void promptForIDAndKey();
    static bool parseAndUpdateSettings(std::vector<std::string> settings);
    static bool parseArgs(int argc, char *argv[]);
    static bool parseArgs(std::vector<std::string> args);
    static bool updateSetting(std::string setting, std::string value);
    static void initializeDataDir();

    static int getClientID();
    static std::string getClientKey();
    static int getClientPort();
    static int getServerTime();
    static bool isStaticRange(std::string fileid);

private:
    static int clientID;
    static std::string clientKey;
    static int serverTimeDelta;

    static std::vector<std::string> rpcServers;
    static std::string imageServer;
    static std::string clientName;
    static std::string clientHost;
    static int clientPort;
    static std::string requestServer;
    static int requestProxyMode;

    static bool forceDirty = false;
    static bool verifyCache = false;
    static bool skipFreeSpaceCheck = false;
    static bool warnNewClient = false;
    static bool useLessMemory = false;
    static bool disableBWM = false;

    static boost::filesystem::path datadir;

    static std::map<std::string, int> staticRanges;
};

#endif //HATH_SETTINGS_H
