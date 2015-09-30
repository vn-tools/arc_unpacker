#include "fmt/west_vision/syg_image_decoder.h"

using namespace au;
using namespace au::fmt::west_vision;

static const bstr magic = "$SYG"_b;

bool SygImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid SygImageDecoder::decode_internal(File &file) const
{
    file.io.seek(0x10);
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    file.io.seek(0x20);
    return pix::Grid(width, height, file.io, pix::Format::BGR888);
}

static auto dummy = fmt::Registry::add<SygImageDecoder>("west-vision/syg");
