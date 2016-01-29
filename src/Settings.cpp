#include "Settings.h"
#include "util.h"

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
std::string Settings::imageServer = "";
std::string Settings::clientName = "";
std::string Settings::clientHost = "";
int Settings::clientPort = 0;
std::string Settings::requestServer = "";
int Settings::requestProxyMode = 0;

bool Settings::loginCredentialsAreSyntaxValid()
{
    std::regex validKey("^[a-zA-Z0-9]{" + std::to_string(CLIENT_KEY_LENGTH) + "}$");
    return clientID > 0 && std::regex_match(clientKey, validKey);
}

bool Settings::loadClientLoginFromFile()
{
    std::ifstream clientLogin("file.txt");

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
            std::cout << "Invalid Client ID. Please try again." << std::endl;
        }
    }
    while (clientID < 1000);

    do
    {
        std::cout << "Enter Client Key: ";
        std::cin >> clientKey;
        if (!loginCredentialsAreSyntaxValid())
            std::cout << "Invalid Client Key, it must be exactly 20 alphanumerical characters. Please try again." << std::endl;
    }
    while (!loginCredentialsAreSyntaxValid());
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
            auto split_str = split(arg.substr(2), '=');
            if (split_str.size() == 2)
                updateSetting(split_str[0], split_str[1]);
            else
                updateSetting(split_str[0], "true");
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

        }
        else if (setting == "name")
        {
            clientName = value;
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

        return true;
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    return false;
}

int Settings::getClientPort()
{
    return clientPort;
}
