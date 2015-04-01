#include <cassert>
#include <stdexcept>
#include "formats/archive.h"
#include "logger.h"

void Archive::add_cli_help(ArgParser &arg_parser) const
{
    for (auto &transformer : transformers)
        transformer->add_cli_help(arg_parser);
}

void Archive::parse_cli_options(ArgParser &arg_parser)
{
    for (auto &transformer : transformers)
        transformer->parse_cli_options(arg_parser);
}

void Archive::add_transformer(std::shared_ptr<Converter> converter)
{
    transformers.push_back(converter);
}

void Archive::unpack_internal(File &, FileSaver &) const
{
    throw std::runtime_error("Unpacking is not supported");
}

void Archive::unpack(File &file, FileSaver &file_saver) const
{
    FileSaverCallback file_saver_proxy([&](const std::shared_ptr<File> file)
    {
        for (auto &transformer : transformers)
            transformer->try_decode(*file);
        file_saver.save(file);
    });
    file.io.seek(0);
    return unpack_internal(file, file_saver_proxy);
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
