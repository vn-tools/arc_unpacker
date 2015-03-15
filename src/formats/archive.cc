#include <cassert>
#include <stdexcept>
#include "formats/archive.h"
#include "logger.h"

void Archive::add_cli_help(ArgParser &arg_parser) const
{
    for (auto &converter : converters)
        converter->add_cli_help(arg_parser);
}

void Archive::parse_cli_options(ArgParser &arg_parser)
{
    for (auto &converter : converters)
        converter->parse_cli_options(arg_parser);
}

void Archive::register_converter(std::unique_ptr<Converter> converter)
{
    converters.push_back(std::move(converter));
}

void Archive::run_converters(File &file) const
{
    for (auto &converter : converters)
        converter->try_decode(file);
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
