#include "fmt/image_decoder.h"
#include "err.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::fmt;

ImageDecoder::~ImageDecoder()
{
}

IDecoder::NamingStrategy ImageDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

void ImageDecoder::unpack(
    io::File &input_file, const FileSaver &file_saver) const
{
    auto output_image = decode(input_file);
    auto output_file = util::file_from_image(output_image, input_file.name);
    // discard any directory information
    output_file->name = io::path(output_file->name).name();
    file_saver.save(std::move(output_file));
}

pix::Image ImageDecoder::decode(io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(file);
}
