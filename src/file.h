#ifndef FILE_H
#define FILE_H
#include <string>
#include "io.h"

class File final
{
public:
    File(const std::string path, const std::string mode);
    File();
    ~File();
    IO &io;
    std::string name;
    void change_extension(const std::string new_extension);
};

#endif
