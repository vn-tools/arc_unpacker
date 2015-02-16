#include <cassert>
#include <stdexcept>
#include "formats/archive.h"
#include "logger.h"

void Archive::add_cli_help(ArgParser &) const
{
}

void Archive::parse_cli_options(ArgParser &)
{
}

void Archive::unpack_internal(File &, FileSaver &) const
{
    throw std::runtime_error("Unpacking is not supported");
}

void Archive::unpack(File &file, FileSaver &file_saver) const
{
    file.io.seek(0);
    return unpack_internal(file, file_saver);
}

bool Archive::try_unpack(File &file, FileSaver &file_saver) const
{
    try
    {
        unpack(file, file_saver);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

Archive::~Archive()
{
}
