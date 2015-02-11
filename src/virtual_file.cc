#include <string>
#include "fs.h"
#include "virtual_file.h"

void VirtualFile::change_extension(const std::string new_extension)
{
    std::string ext_copy(new_extension);
    while (ext_copy.length() > 0 && ext_copy[0] == '.')
        ext_copy.erase(0, 1);

    std::string base_name(basename(name));

    bool all_dots = true;
    for (auto &c : base_name)
        all_dots &= c == '.';
    if (all_dots)
    {
        return;
    }

    std::string new_name(dirname(name));
    if (new_name.length() > 0)
    {
        if (new_name.back() != '/' && new_name.back() != '\\')
            new_name += "/";
    }

    size_t pos = base_name.rfind(".");
    if (pos == std::string::npos)
        new_name += base_name + "." + ext_copy;
    else
        new_name += base_name.substr(0, pos) + "." + ext_copy;

    name = new_name;
}
