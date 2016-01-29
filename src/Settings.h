#ifndef HATH_SETTINGS_H
#define HATH_SETTINGS_H

#include <string>
#include <vector>

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
    static bool parseArgs(int argc, char *argv[]);
    static bool parseArgs(std::vector<std::string> args);
    static bool updateSetting(std::string setting, std::string value);

    static int getClientPort();

private:
    static int clientID;
    static std::string clientKey;
    static int serverTimeDelta;

    static std::string imageServer;
    static std::string clientName;
    static std::string clientHost;
    static int clientPort;
    static std::string requestServer;
    static int requestProxyMode;
};

#endif //HATH_SETTINGS_H
