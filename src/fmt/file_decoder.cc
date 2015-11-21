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
    return std::make_unique<SiblingNamingStrategy>();
}

void FileDecoder::register_cli_options(ArgParser &) const
{
}

void FileDecoder::parse_cli_options(const ArgParser &)
{
}

bool FileDecoder::is_recognized(io::File &file) const
{
    try
    {
        file.stream.seek(0);
        return is_recognized_impl(file);
    }
    catch (...)
    {
        return false;
    }
}

void FileDecoder::unpack(
    io::File &input_file, const FileSaver &file_saver) const
{
    auto output_file = decode(input_file);
    output_file->name
        = boost::filesystem::path(output_file->name).filename().string();
    file_saver.save(std::move(output_file));
}

std::unique_ptr<io::File> FileDecoder::decode(io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(file);
}
