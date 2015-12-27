#include "fmt/leaf/leafpack_group/lf3_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAF64K\x00"_b;

bool Lf3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Lf3ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_u16_le();
    const auto height = input_file.stream.read_u16_le();

    const auto data_pos = input_file.stream.read_u32_le();

    input_file.stream.seek(data_pos);
    const auto data = common::custom_lzss_decompress(
        input_file.stream.read_to_eof(), width * height * 2);
    res::Image image(width, height, data, res::PixelFormat::BGR555X);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lf3ImageDecoder>("leaf/lf3");
