#include <cassert>
#include <string>
#include "io.h"
#include "logger.h"
#include "string_ex.h"
#include "virtual_file.h"

typedef struct
{
    std::string name;
} Internals;

VirtualFile *virtual_file_create()
{
    VirtualFile *file = new VirtualFile;
    assert(file != nullptr);

    file->io = io_create_empty();
    assert(file->io != nullptr);

    file->internals = new Internals;
    return file;
}

void virtual_file_destroy(VirtualFile *file)
{
    assert(file != nullptr);
    io_destroy(file->io);
    delete (Internals*)file->internals;
    delete file;
}

void virtual_file_change_extension(VirtualFile *file, const char *new_ext)
{
    assert(file != nullptr);
    Internals *internals = ((Internals*)file->internals);
    std::string old_name = internals->name;
    if (old_name == "")
        return;

    size_t pos = old_name.rfind(".");
    if (pos == std::string::npos)
    {
        internals->name = old_name + "." + std::string(new_ext);
    }
    else
    {
        internals->name = old_name.substr(0, pos) + "." + std::string(new_ext);
    }
}

const char *virtual_file_get_name(VirtualFile *file)
{
    return ((Internals*)file->internals)->name.c_str();
}

void virtual_file_set_name(VirtualFile *file, const char *new_name)
{
    assert(file != nullptr);
    ((Internals*)file->internals)->name = std::string(new_name);
}
