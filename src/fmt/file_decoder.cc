#include "fmt/file_decoder.h"
#include "err.h"
#include "fmt/naming_strategies.h"

using namespace au;
using namespace au::fmt;

FileDecoder::~FileDecoder()
{
}

std::unique_ptr<INamingStrategy> FileDecoder::naming_strategy() const
{
    return std::unique_ptr<INamingStrategy>(new SiblingNamingStrategy);
}

void FileDecoder::register_cli_options(ArgParser &) const
{
}

void FileDecoder::parse_cli_options(const ArgParser &)
{
}

bool FileDecoder::is_recognized(File &file) const
{
    try
    {
        file.io.seek(0);
        return is_recognized_internal(file);
    }
    catch (...)
    {
        return false;
    }
}

void FileDecoder::unpack(
    File &input_file, FileSaver &file_saver, bool recurse) const
{
    auto output_file = decode(input_file);
    output_file->name
        = boost::filesystem::path(output_file->name).filename().string();
    file_saver.save(std::move(output_file));
}

std::unique_ptr<File> FileDecoder::decode(File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();

    file.io.seek(0);
    return decode_internal(file);
}
