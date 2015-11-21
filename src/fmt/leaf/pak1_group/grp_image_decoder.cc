#include "fmt/leaf/pak1_group/grp_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static int detect_version(File &file)
{
    int version = 1;
    file.stream.seek(0);
    size_t width = file.stream.read_u16_le();
    size_t height = file.stream.read_u16_le();
    if (!width && !height)
    {
        width = file.stream.read_u16_le();
        height = file.stream.read_u16_le();
        version = 2;
    }
    return width * height + version * 4 == file.stream.size() ? version : -1;
}

static pix::Grid decode_pixels(File &file)
{
    const auto version = detect_version(file);
    file.stream.seek(version == 1 ? 0 : 4);
    const auto width = file.stream.read_u16_le();
    const auto height = file.stream.read_u16_le();
    const auto data = file.stream.read(width * height);
    return pix::Grid(width, height, data, pix::Format::Gray8);
}

static pix::Palette decode_palette(File &file)
{
    file.stream.seek(0);
    const auto count = file.stream.read_u16_le();
    auto palette = pix::Palette(256);
    for (auto i : util::range(count))
    {
        auto index = file.stream.read_u8();
        palette[index].a = 0xFF;
        palette[index].b = file.stream.read_u8();
        palette[index].g = file.stream.read_u8();
        palette[index].r = file.stream.read_u8();
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
        mask_file->stream.seek(0);
        auto mask_data = mask_file->stream.read_to_eof();
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
