#include "dec/leaf/leafpack_group/lf2_image_decoder.h"
#include "dec/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "LEAF256\x00"_b;

bool Lf2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Lf2ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    auto width = input_file.stream.read_le<u16>();
    auto height = input_file.stream.read_le<u16>();
    auto size_orig = width * height;

    input_file.stream.seek(0x16);
    auto color_count = input_file.stream.read_le<u16>();
    res::Palette palette(
        color_count, input_file.stream, res::PixelFormat::BGR888);

    const auto data = common::custom_lzss_decompress(
        input_file.stream.read_to_eof(), width * height);
    res::Image image(width, height, data, palette);
    image.flip_vertically();
    return image;
}

static auto _ = dec::register_decoder<Lf2ImageDecoder>("leaf/lf2");
