#include <cassert>
#include <string>
#include "buffered_io.h"
#include "logger.h"
#include "string_ex.h"
#include "virtual_file.h"

void VirtualFile::change_extension(const std::string new_extension)
{
    if (name == "")
        return;

    std::string ext_copy = new_extension;
    while (ext_copy.length() > 0 && ext_copy[0] == '.')
        ext_copy.erase(0, 1);

    size_t pos = name.rfind(".");
    if (pos == std::string::npos)
        name += "." + ext_copy;
    else
        name = name.substr(0, pos) + "." + ext_copy;
}
