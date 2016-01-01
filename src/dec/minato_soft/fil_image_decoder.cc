#include "dec/minato_soft/fil_image_decoder.h"

using namespace au;
using namespace au::dec::minato_soft;

bool FilImageDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    return 8 + width * height == input_file.stream.size();
}

res::Image FilImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto data = input_file.stream.read(width * height);
    return res::Image(width, height, data, res::PixelFormat::Gray8);
}

static auto _ = dec::register_decoder<FilImageDecoder>("minato-soft/fil");
