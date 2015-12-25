#include "fmt/image_decoder.h"
#include "err.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::fmt;

IDecoder::NamingStrategy ImageDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

void ImageDecoder::unpack(
    const Logger &logger,
    io::File &input_file,
    const FileSaver &file_saver) const
{
    auto output_image = decode(logger, input_file);
    auto output_file = util::file_from_image(output_image, input_file.path);
    // discard any directory information
    output_file->path = output_file->path.name();
    file_saver.save(std::move(output_file));
}

res::Image ImageDecoder::decode(const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
