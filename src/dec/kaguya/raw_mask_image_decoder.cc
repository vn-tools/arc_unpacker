#include "dec/kaguya/raw_mask_image_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

bool RawMaskImageDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    return input_file.stream.size() == width * height + 8;
}

res::Image RawMaskImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    return res::Image(
        width,
        height,
        input_file.stream.read_to_eof(),
        res::PixelFormat::Gray8);
}

static auto _
    = dec::register_decoder<RawMaskImageDecoder>("kaguya/raw-mask");
