#include <cassert>
#include <stdexcept>
#include "formats/archive.h"
#include "logger.h"

void Archive::add_cli_help(__attribute__((unused)) ArgParser &arg_parser) const
{
}

void Archive::parse_cli_options(__attribute__((unused)) ArgParser &arg_parser)
{
}

void Archive::unpack_internal(
    __attribute__((unused)) IO &arc_io,
    __attribute__((unused)) OutputFiles &output_files) const
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
