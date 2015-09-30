#include "fmt/lizsoft/sotes_image_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lizsoft;

bool SotesImageDecoder::is_recognized_internal(File &file) const
{
    file.io.seek(0x438);
    auto a = file.io.read_u32_le();
    file.io.seek(0x448);
    auto b = file.io.read_u32_le();
    file.io.seek(0x450);
    auto c = file.io.read_u32_le();
    return a - b == 0x2711 && c - b <= 0x80;
}

pix::Grid SotesImageDecoder::decode_internal(File &file) const
{
    file.io.seek(0x448);
    auto base = file.io.read_u32_le();

    file.io.seek(0x450);
    auto pixel_data_offset = 0x458 + file.io.read_u32_le() - base;

    file.io.seek(0x430);
    auto depth = file.io.read_u16_le() - base;

    file.io.seek(0x440);
    file.io.seek(4 + 4 * (file.io.read_u32_le() - base));
    auto width = file.io.read_u32_le() - base;

    file.io.seek(0x18);
    file.io.seek(0x420 + 4 * (file.io.read_u32_le() - base));
    auto height = file.io.read_u32_le() - base;

    file.io.seek(0x20);
    pix::Palette palette(256, file.io, pix::Format::BGR888X);

    file.io.seek(pixel_data_offset);
    auto data = file.io.read(width * height * (depth >> 3));

    std::unique_ptr<pix::Grid> pixels;
    if (depth == 8)
        pixels.reset(new pix::Grid(width, height, data, palette));
    else if (depth == 24)
        pixels.reset(new pix::Grid(width, height, data, pix::Format::BGR888));
    else
        throw err::UnsupportedBitDepthError(depth);

    pixels->flip();
    return *pixels;
}

static auto dummy = fmt::Registry::add<SotesImageDecoder>("lizsoft/sotes");
