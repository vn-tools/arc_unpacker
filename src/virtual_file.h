#ifndef VIRTUAL_FILE_H
#define VIRTUAL_FILE_H
#include <string>
#include "buffered_io.h"

class VirtualFile final
{
public:
    BufferedIO io;
    std::string name;
    void change_extension(const std::string new_extension);
};

#endif
