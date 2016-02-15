#include "Settings.h"
#include "util.h"
#include "Out.h"
#include "HathClient.h"

#include <iterator>
#include <regex>
#include <fstream>

const std::string Settings::CLIENT_VERSION = "1.2.6";
const std::string Settings::CLIENT_API_URL = "http://rpc.hentaiathome.net/clientapi.php?";
const std::string Settings::DATA_FILENAME_CLIENT_LOGIN = "client_login";
const std::string Settings::DATA_FILENAME_LASTHIT_HISTORY = "lasthit_history";
int Settings::clientID = 0;
std::string Settings::clientKey = "";
int Settings::serverTimeDelta = 0;
std::vector<std::string> Settings::rpcServers;
std::string Settings::imageServer = "";
std::string Settings::clientName = "";
std::string Settings::clientHost = "";
int Settings::clientPort = 0;
std::string Settings::requestServer = "";
int Settings::requestProxyMode = 0;
bool Settings::forceDirty = false;
bool Settings::verifyCache = false;
bool Settings::skipFreeSpaceCheck = false;
bool Settings::warnNewClient = false;
bool Settings::useLessMemory = false;
bool Settings::disableBWM = false;
boost::filesystem::path Settings::datadir;
std::map<std::string, int> Settings::staticRanges;

bool Settings::loginCredentialsAreSyntaxValid()
{
    std::regex validKey("^[a-zA-Z0-9]{" + std::to_string(CLIENT_KEY_LENGTH) + "}$");
    return clientID > 0 && std::regex_match(clientKey, validKey);
}

bool Settings::loadClientLoginFromFile()
{
    std::ifstream clientLogin(path(datadir / DATA_FILENAME_CLIENT_LOGIN).c_str());

    if (clientLogin.bad())
        return false;

    std::string filecontent((std::istreambuf_iterator<char>(clientLogin)),
                    std::istreambuf_iterator<char>());
    if (filecontent.size())
    {
        auto spl = split(filecontent, '-', 2);
        if (spl.size() == 2)
        {
            clientID = std::stoi(spl[0]);
            clientKey = spl[1];
            return true;
        }
    }

    return false;
}

void Settings::promptForIDAndKey()
{
    Out::info("Before you can use this client, you will have to register it at http://hentaiathome.net/");
    Out::info("IMPORTANT: YOU NEED A SEPARATE IDENT FOR EACH CLIENT YOU WANT TO RUN.");
    Out::info("DO NOT ENTER AN IDENT THAT WAS ASSIGNED FOR A DIFFERENT CLIENT.");
    Out::info("After registering, enter your ID and Key below to start your client.");
    Out::info("(You will only have to do this once.)\n");

    clientID = 0;
    clientKey = "";

    do
    {
        std::cout << "Enter Client ID: ";
        std::cin >> clientID;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Out::warning("Invalid Client ID. Please try again.");
        }
    }
    while (clientID < 1000);

    do
    {
        std::cout << "Enter Client Key: ";
        std::cin >> clientKey;
        if (!loginCredentialsAreSyntaxValid())
            Out::warning("Invalid Client Key, it must be exactly 20 alphanumerical characters. Please try again.");
    }
    while (!loginCredentialsAreSyntaxValid());

    putStringFileContents(path(datadir / DATA_FILENAME_CLIENT_LOGIN).string(),
                          (std::to_string(clientID) + "-" + clientKey));
}

bool Settings::parseAndUpdateSettings(std::vector<std::string> settings)
{
    if (!settings.size())
        return false;

    for (std::string s : settings)
    {
        auto spl = split(s, '=', 2);
        if (spl.size() == 2)
            updateSetting(spl[0], spl[1]);
    }

    return true;
}

// note that these settings will currently be overwritten by any equal ones read from the server, so it should not be used to override server-side settings.
bool Settings::parseArgs(int argc, char *argv[])
{
    std::vector<std::string> args(argv+1, argv+argc);
    return parseArgs(args);
}

bool Settings::parseArgs(std::vector<std::string> args)
{
    if (args.size() == 0)
        return false;

    for (auto const& arg: args)
    {
        if (arg.find("--") == 0)
        {
            auto split_str = split(arg.substr(2), '=', 2);
            if (split_str.size() == 2)
                updateSetting(split_str[0], split_str[1]);
            else
                updateSetting(split_str[0], "true");
        }
        else
        {
            Out::warning("Invalid command argument: " + arg);
        }
    }

    return true;
}

bool Settings::updateSetting(std::string setting, std::string value)
{
    setting = replace(setting, "-", "_");

    try
    {
        if (setting == "min_client_build")
        {
            if (std::stoi(value) > CLIENT_BUILD)
                HathClient::dieWithError("Your client is too old to connect to the Hentai@Home Network. Please download the new version of the client from http://hentaiathome.net/");
        }
        if (setting == "cur_client_build")
        {
            if (std::stoi(value) > CLIENT_BUILD)
                warnNewClient = true;
        }
        else if (setting == "server_time")
        {
            serverTimeDelta = (std::stoi(value) - static_cast<int>(currentTime()));
            Out::debug("Setting altered: serverTimeDelta=" + std::to_string(serverTimeDelta));
        }
        else if (setting == "rpc_server_ip")
        {
            auto spl = split(value, ';');
            for (std::string s : spl)
                rpcServers.push_back(s);
        }
        else if (setting == "image_server")
        {
            imageServer = value;
        }
        else if (setting == "name")
        {
            clientName = value;
        }
        else if (setting == "host")
        {
            clientHost = value;
        }
        else if (setting == "port")
        {
            if (clientPort == 0)
                clientPort = std::stoi(value);
        }
        else if (setting == "request_server")
        {
            requestServer = value;
        }
        else if (setting == "request_proxy_mode")
        {
            requestProxyMode = std::stoi(value);
        }
        else if (setting == "static_ranges")
        {
            auto spl = split(value, ';');
            for (std::string s : spl)
            {
                if (s.size() == 4)
                    staticRanges.insert(std::make_pair(s, 1));
            }
        }
        else
        {
            Out::warning("Unknown setting " + setting + " = " + value);
            return false;
        }

        Out::debug("Setting altered: " + setting + "=" + value);
        return true;
    }
    catch (std::exception e)
    {
        Out::warning("Failed parsing setting " + setting + " = " + value);
    }

    return false;
}

void Settings::initializeDataDir()
{
    datadir = checkAndCreateDir("data");
}

int Settings::getClientID()
{
    return clientID;
}

std::string Settings::getClientKey()
{
    return clientKey;
}

int Settings::getClientPort()
{
    return clientPort;
}

int Settings::getServerTime()
{
    return static_cast<int>(currentTime() + serverTimeDelta);
}

bool Settings::isStaticRange(std::string fileid)
{
    if (staticRanges.size())
        return (staticRanges.find(fileid.substr(0, 4)) != staticRanges.end());

    return false;
}
