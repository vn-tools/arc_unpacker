#include "fmt/leaf/grp_image_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static pix::Grid decode_pixels(File &file)
{
    file.io.seek(0);
    auto width = file.io.read_u16_le();
    auto height = file.io.read_u16_le();
    auto data =  file.io.read(width * height);
    return pix::Grid(width, height, data, pix::Format::Gray8);
}

static pix::Palette decode_palette(File &file)
{
    file.io.seek(0);
    auto count = file.io.read_u16_le();
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

bool GrpImageDecoder::is_recognized_internal(File &file) const
{
    size_t expected_size = file.io.read_u16_le() * file.io.read_u16_le() + 4;
    return expected_size == file.io.size();
}

pix::Grid GrpImageDecoder::decode(File &file, File &palette_file) const
{
    auto pixels = decode_pixels(file);
    auto palette = decode_palette(palette_file);
    pixels.apply_palette(palette);
    return pixels;
}

pix::Grid GrpImageDecoder::decode_internal(File &file) const
{
    return decode_pixels(file);
}

static auto dummy = fmt::Registry::add<GrpImageDecoder>("leaf/grp");
