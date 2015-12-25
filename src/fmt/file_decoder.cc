#include "fmt/file_decoder.h"
#include "err.h"

using namespace au;
using namespace au::fmt;

IDecoder::NamingStrategy FileDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

void FileDecoder::unpack(
    const Logger &logger,
    io::File &input_file,
    const FileSaver &file_saver) const
{
    auto output_file = decode(logger, input_file);
    // discard any directory information
    output_file->path = output_file->path.name();
    file_saver.save(std::move(output_file));
}

std::unique_ptr<io::File> FileDecoder::decode(
    const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
