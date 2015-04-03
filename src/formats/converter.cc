#include <cassert>
#include <stdexcept>
#include "formats/converter.h"

void Converter::unpack_internal(File &file, FileSaver &file_saver) const
{
    // TODO:
    // change decode() signature to
    // std::unique_ptr<File> decode(const File &input)
    // so that we don't need to copy File here all the time
    std::shared_ptr<File> file_copy(new File);
    file.io.seek(0);
    file_copy->name = file.name;
    file_copy->io.write_from_io(file.io);
    decode(*file_copy);
    file_copy->name
        = boost::filesystem::path(file_copy->name).filename().string();
    file_saver.save(file_copy);
}

void Converter::add_cli_help(ArgParser &) const
{
}

void Converter::parse_cli_options(const ArgParser &)
{
}

FileNamingStrategy Converter::get_file_naming_strategy() const
{
    return FileNamingStrategy::Sibling;
}

void Converter::decode_internal(File &) const
{
    throw std::runtime_error("Decoding is not supported");
}

void Converter::decode(File &target_file) const
{
    target_file.io.seek(0);
    decode_internal(target_file);
}

bool Converter::try_decode(File &target_file) const
{
    try
    {
        decode(target_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

Converter::~Converter()
{
}
