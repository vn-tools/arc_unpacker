#ifndef VIRTUAL_FILE_H
#define VIRTUAL_FILE_H
#include <string>
#include "io.h"

class VirtualFile final
{
public:
    IO &io;
    std::string name;
    VirtualFile();
    ~VirtualFile();
    void change_extension(const std::string new_extension);
};

#endif
