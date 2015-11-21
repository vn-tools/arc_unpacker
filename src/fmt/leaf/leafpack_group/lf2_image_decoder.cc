#include "fmt/leaf/leafpack_group/lf2_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAF256\x00"_b;

bool Lf2ImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid Lf2ImageDecoder::decode_impl(File &file) const
{
    file.stream.seek(magic.size());
    file.stream.skip(4);
    auto width = file.stream.read_u16_le();
    auto height = file.stream.read_u16_le();
    auto size_orig = width * height;

    file.stream.seek(0x16);
    auto color_count = file.stream.read_u16_le();
    pix::Palette palette(color_count, file.stream, pix::Format::BGR888);

    const auto data = common::custom_lzss_decompress(
        file.stream.read_to_eof(), width * height);
    pix::Grid image(width, height, data, palette);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lf2ImageDecoder>("leaf/lf2");
