#include <cassert>
#include <stdexcept>
#include "formats/archive.h"
#include "logger.h"

void Archive::parse_cli_options(ArgParser &arg_parser)
{
}

void Archive::add_cli_help(ArgParser &arg_parser)
{
}

bool Archive::unpack_internal(IO *arc_io, OutputFiles *output_files)
{
    throw std::runtime_error("Unpacking is not supported");
}

bool Archive::unpack(IO *arc_io, OutputFiles *output_files)
{
    io_seek(arc_io, 0);
    return unpack_internal(arc_io, output_files);
}

Archive::~Archive()
{
}
