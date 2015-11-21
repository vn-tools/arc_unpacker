#include "fmt/leaf/leafpack_group/lc3_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAFC64\x00"_b;

bool Lc3ImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid Lc3ImageDecoder::decode_impl(File &file) const
{
    file.stream.seek(magic.size());
    file.stream.skip(4);
    const auto width = file.stream.read_u16_le();
    const auto height = file.stream.read_u16_le();

    const auto alpha_pos = file.stream.read_u32_le();
    const auto color_pos = file.stream.read_u32_le();

    file.stream.seek(color_pos);
    const auto color_data = common::custom_lzss_decompress(
        file.stream.read_to_eof(), width * height * 2);
    pix::Grid image(width, height, color_data, pix::Format::BGR555X);

    file.stream.seek(alpha_pos);
    auto mask_data = common::custom_lzss_decompress(
        file.stream.read(color_pos - alpha_pos), width * height);
    for (auto &c : mask_data)
        c <<= 3;
    pix::Grid mask(width, height, mask_data, pix::Format::Gray8);

    image.apply_mask(mask);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lc3ImageDecoder>("leaf/lc3");
