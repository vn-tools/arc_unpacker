#include "err.h"
#include "fmt/converter.h"

using namespace au;
using namespace au::fmt;

void Converter::unpack(
    File &input_file, FileSaver &file_saver, bool recurse) const
{
    auto output_file = decode(input_file);
    output_file->name
        = boost::filesystem::path(output_file->name).filename().string();
    file_saver.save(std::move(output_file));
}

void Converter::register_cli_options(ArgParser &) const
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
    if (!is_recognized(file))
        throw err::RecognitionError();

    file.io.seek(0);
    return decode_internal(file);
}

Converter::~Converter()
{
}
