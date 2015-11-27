#include "fmt/minato_soft/fil_image_decoder.h"

using namespace au;
using namespace au::fmt::minato_soft;

bool FilImageDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    return 8 + width * height == input_file.stream.size();
}

pix::Image FilImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto data = input_file.stream.read(width * height);
    return pix::Image(width, height, data, pix::PixelFormat::Gray8);
}

static auto dummy = fmt::register_fmt<FilImageDecoder>("minato-soft/fil");
