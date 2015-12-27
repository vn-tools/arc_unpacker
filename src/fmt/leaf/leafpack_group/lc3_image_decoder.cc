#include "fmt/leaf/leafpack_group/lc3_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAFC64\x00"_b;

bool Lc3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Lc3ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_u16_le();
    const auto height = input_file.stream.read_u16_le();

    const auto alpha_pos = input_file.stream.read_u32_le();
    const auto color_pos = input_file.stream.read_u32_le();

    input_file.stream.seek(color_pos);
    const auto color_data = common::custom_lzss_decompress(
        input_file.stream.read_to_eof(), width * height * 2);
    res::Image image(width, height, color_data, res::PixelFormat::BGR555X);

    input_file.stream.seek(alpha_pos);
    auto mask_data = common::custom_lzss_decompress(
        input_file.stream.read(color_pos - alpha_pos), width * height);
    for (auto &c : mask_data)
        c <<= 3;
    res::Image mask(width, height, mask_data, res::PixelFormat::Gray8);

    image.apply_mask(mask);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lc3ImageDecoder>("leaf/lc3");
