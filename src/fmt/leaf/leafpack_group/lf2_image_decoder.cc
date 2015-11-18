#include "fmt/leaf/leafpack_group/lf2_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAF256\x00"_b;

bool Lf2ImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid Lf2ImageDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());
    file.io.skip(4);
    auto width = file.io.read_u16_le();
    auto height = file.io.read_u16_le();
    auto size_orig = width * height;

    file.io.seek(0x16);
    auto color_count = file.io.read_u16_le();
    pix::Palette palette(color_count, file.io, pix::Format::BGR888);

    const auto data = common::custom_lzss_decompress(
        file.io.read_to_eof(), width * height);
    pix::Grid image(width, height, data, palette);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lf2ImageDecoder>("leaf/lf2");
