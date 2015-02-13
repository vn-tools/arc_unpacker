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

void Archive::unpack_internal(VirtualFile &, OutputFiles &) const
{
    throw std::runtime_error("Unpacking is not supported");
}

void Archive::unpack(VirtualFile &file, OutputFiles &output_files) const
{
    file.io.seek(0);
    return unpack_internal(file, output_files);
}

Archive::~Archive()
{
}
