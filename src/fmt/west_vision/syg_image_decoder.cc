#include "fmt/west_vision/syg_image_decoder.h"

using namespace au;
using namespace au::fmt::west_vision;

static const bstr magic = "$SYG"_b;

bool SygImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Image SygImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(0x10);
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    input_file.stream.seek(0x20);
    return pix::Image(width, height, input_file.stream, pix::Format::BGR888);
}

static auto dummy = fmt::register_fmt<SygImageDecoder>("west-vision/syg");
