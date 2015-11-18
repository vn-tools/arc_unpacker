#include "fmt/leaf/pak1_group/grp_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static int detect_version(File &file)
{
    int version = 1;
    file.io.seek(0);
    size_t width = file.io.read_u16_le();
    size_t height = file.io.read_u16_le();
    if (!width && !height)
    {
        width = file.io.read_u16_le();
        height = file.io.read_u16_le();
        version = 2;
    }
    return width * height + version * 4 == file.io.size() ? version : -1;
}

static pix::Grid decode_pixels(File &file)
{
    const auto version = detect_version(file);
    file.io.seek(version == 1 ? 0 : 4);
    const auto width = file.io.read_u16_le();
    const auto height = file.io.read_u16_le();
    const auto data = file.io.read(width * height);
    return pix::Grid(width, height, data, pix::Format::Gray8);
}

static pix::Palette decode_palette(File &file)
{
    file.io.seek(0);
    const auto count = file.io.read_u16_le();
    auto palette = pix::Palette(256);
    for (auto i : util::range(count))
    {
        auto index = file.io.read_u8();
        palette[index].a = 0xFF;
        palette[index].b = file.io.read_u8();
        palette[index].g = file.io.read_u8();
        palette[index].r = file.io.read_u8();
    }
    return palette;
}

bool GrpImageDecoder::is_recognized_impl(File &file) const
{
    return detect_version(file) > 0;
}

pix::Grid GrpImageDecoder::decode(
    File &file,
    std::shared_ptr<File> palette_file,
    std::shared_ptr<File> mask_file) const
{
    auto image = decode_pixels(file);
    if (palette_file)
    {
        const auto palette = decode_palette(*palette_file);
        image.apply_palette(palette);
    }
    if (mask_file)
    {
        mask_file->io.seek(0);
        auto mask_data = mask_file->io.read_to_eof();
        for (auto &c : mask_data)
            c ^= 0xFF;
        image.apply_mask(pix::Grid(
            image.width(), image.height(), mask_data, pix::Format::Gray8));
    }
    return image;
}

pix::Grid GrpImageDecoder::decode_impl(File &file) const
{
    return decode_pixels(file);
}

static auto dummy = fmt::register_fmt<GrpImageDecoder>("leaf/grp");
