#include "fmt/west_vision/syg_image_decoder.h"

using namespace au;
using namespace au::fmt::west_vision;

static const bstr magic = "$SYG"_b;

bool SygImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid SygImageDecoder::decode_impl(File &file) const
{
    file.stream.seek(0x10);
    auto width = file.stream.read_u32_le();
    auto height = file.stream.read_u32_le();
    file.stream.seek(0x20);
    return pix::Grid(width, height, file.stream, pix::Format::BGR888);
}

static auto dummy = fmt::register_fmt<SygImageDecoder>("west-vision/syg");
