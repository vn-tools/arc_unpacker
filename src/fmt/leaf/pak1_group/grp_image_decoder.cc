#include "fmt/leaf/pak1_group/grp_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static int detect_version(io::File &input_file)
{
    int version = 1;
    input_file.stream.seek(0);
    size_t width = input_file.stream.read_u16_le();
    size_t height = input_file.stream.read_u16_le();
    if (!width && !height)
    {
        width = input_file.stream.read_u16_le();
        height = input_file.stream.read_u16_le();
        version = 2;
    }
    if (width * height + version * 4 == input_file.stream.size())
        return version;
    return -1;
}

static res::Image decode_image(io::File &input_file)
{
    const auto version = detect_version(input_file);
    input_file.stream.seek(version == 1 ? 0 : 4);
    const auto width = input_file.stream.read_u16_le();
    const auto height = input_file.stream.read_u16_le();
    const auto data = input_file.stream.read(width * height);
    return res::Image(width, height, data, res::PixelFormat::Gray8);
}

static res::Palette decode_palette(io::File &input_file)
{
    input_file.stream.seek(0);
    const auto count = input_file.stream.read_u16_le();
    auto palette = res::Palette(256);
    for (auto i : util::range(count))
    {
        auto index = input_file.stream.read_u8();
        palette[index].a = 0xFF;
        palette[index].b = input_file.stream.read_u8();
        palette[index].g = input_file.stream.read_u8();
        palette[index].r = input_file.stream.read_u8();
    }
    return palette;
}

bool GrpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return detect_version(input_file) > 0;
}

res::Image GrpImageDecoder::decode(
    io::File &input_file,
    std::shared_ptr<io::File> palette_file,
    std::shared_ptr<io::File> mask_file) const
{
    auto image = decode_image(input_file);
    if (palette_file)
    {
        const auto palette = decode_palette(*palette_file);
        image.apply_palette(palette);
    }
    if (mask_file)
    {
        mask_file->stream.seek(0);
        auto mask_data = mask_file->stream.read_to_eof();
        for (auto &c : mask_data)
            c ^= 0xFF;
        image.apply_mask(res::Image(
            image.width(), image.height(), mask_data, res::PixelFormat::Gray8));
    }
    return image;
}

res::Image GrpImageDecoder::decode_impl(io::File &input_file) const
{
    return decode_image(input_file);
}

static auto dummy = fmt::register_fmt<GrpImageDecoder>("leaf/grp");
