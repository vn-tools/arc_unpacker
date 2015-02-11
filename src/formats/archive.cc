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

void Archive::unpack_internal(IO &, OutputFiles &) const
{
    throw std::runtime_error("Unpacking is not supported");
}

void Archive::unpack(IO &arc_io, OutputFiles &output_files) const
{
    arc_io.seek(0);
    return unpack_internal(arc_io, output_files);
}

Archive::~Archive()
{
}
