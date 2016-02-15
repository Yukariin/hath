#ifndef HATH_FILE_H
#define HATH_FILE_H

#include <string>
#include <boost/filesystem/path.hpp>

class File
{
public:
    File(std::string hash, int size, int xres, int yres, std::string type);

    boost::filesystem::path getLocalFilePath();

    std::string getFileid();
    int getSize();

    // static stuff
    static bool isValidHVFileid(std::string fileid);
    static std::shared_ptr<File> getHVFileFromFileid(std::string fileid);

private:
    std::string hash;
    int size;
    int xres;
    int yres;
    std::string type;
};

#endif //HATH_FILE_H
