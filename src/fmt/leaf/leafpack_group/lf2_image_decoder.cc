#include "fmt/leaf/leafpack_group/lf2_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAF256\x00"_b;

bool Lf2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Image Lf2ImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    auto width = input_file.stream.read_u16_le();
    auto height = input_file.stream.read_u16_le();
    auto size_orig = width * height;

    input_file.stream.seek(0x16);
    auto color_count = input_file.stream.read_u16_le();
    pix::Palette palette(color_count, input_file.stream, pix::Format::BGR888);

    const auto data = common::custom_lzss_decompress(
        input_file.stream.read_to_eof(), width * height);
    pix::Image image(width, height, data, palette);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lf2ImageDecoder>("leaf/lf2");
