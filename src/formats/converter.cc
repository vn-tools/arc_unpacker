#include <cassert>
#include <stdexcept>
#include "formats/converter.h"

void Converter::unpack_internal(File &input_file, FileSaver &file_saver) const
{
    auto output_file = decode(input_file);
    output_file->name
        = boost::filesystem::path(output_file->name).filename().string();
    file_saver.save(std::move(output_file));
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

std::unique_ptr<File> Converter::decode(File &file) const
{
    file.io.seek(0);
    return decode_internal(file);
}

Converter::~Converter()
{
}
