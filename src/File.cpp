#include "File.h"

#include "util.h"

#include <regex>

File::File(std::string hash, int size, int xres, int yres, std::string type) :
        hash(hash), size(size), xres(xres), yres(yres), type(type)
{

}

boost::filesystem::path File::getLocalFilePath()
{
    return boost::filesystem::path(hash.substr(0, 2) / getFileid());
}

std::string File::getFileid() {
    std::ostringstream ss;
    ss << hash << "-"
    << size << "-"
    << xres << "-"
    << yres << "-"
    << type << "-";
    return ss.str();
}

int File::getSize()
{
    return size;
}

bool File::isValidHVFileid(std::string fileid)
{
    std::regex validFileid("^[a-f0-9]{40}-[0-9]{1,8}-[0-9]{1,5}-[0-9]{1,5}-((jpg)|(png)|(gif)|(wbm))$");
    return std::regex_match(fileid.c_str(), validFileid);
}

File File::getHVFileFromFileid(std::string fileid)
{
    if (isValidHVFileid(fileid))
    {
        auto fileidParts = split(fileid, '-');
        std::string hash = fileidParts[0];
        int size = std::stoi(fileidParts[1]);
        int xres = std::stoi(fileidParts[2]);
        int yres = std::stoi(fileidParts[3]);
        std::string type = fileidParts[4];

        return File(hash, size, xres, yres, type);
    }
}
