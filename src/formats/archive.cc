#include <cassert>
#include <stdexcept>
#include "formats/archive.h"
#include "logger.h"

void Archive::add_cli_help(ArgParser &arg_parser) const
{
    for (auto &converter : converters)
        converter->add_cli_help(arg_parser);
    for (auto &archive : archives)
    {
        if (archive != this)
            archive->add_cli_help(arg_parser);
    }
}

void Archive::parse_cli_options(const ArgParser &arg_parser)
{
    for (auto &converter : converters)
        converter->parse_cli_options(arg_parser);
    for (auto &archive : archives)
    {
        if (archive != this)
            archive->parse_cli_options(arg_parser);
    }
}

void Archive::add_transformer(Converter *converter)
{
    converters.push_back(converter);
}

void Archive::add_transformer(Archive *archive)
{
    archives.push_back(archive);
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

void Archive::unpack(File &file, FileSaver &file_saver) const
{
    FileSaverCallback file_saver_proxy;

    //every file should be passed through registered converters and archives
    file_saver_proxy.set_callback([&](std::shared_ptr<File> saved_file)
    {
        //nested archives should have OUTER FILE/INNER FILE names
        std::string prefix = saved_file->name + "/";
        FileSaverCallback nested_proxy([&](std::shared_ptr<File> nested_file)
        {
            nested_file->name = prefix + nested_file->name;
            file_saver_proxy.save(nested_file);
        });

        for (auto &archive : archives)
        {
            if (archive->try_unpack(*saved_file, nested_proxy))
                return;
        }

        for (auto &converter : converters)
            converter->try_decode(*saved_file);
        file_saver.save(saved_file);
    });

    file.io.seek(0);
    return unpack_internal(file, file_saver_proxy);
}

void Archive::unpack_internal(File &, FileSaver &) const
{
    throw std::runtime_error("Unpacking is not supported");
}

Archive::~Archive()
{
}
