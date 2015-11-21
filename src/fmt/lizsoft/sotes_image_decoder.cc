#include "fmt/lizsoft/sotes_image_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lizsoft;

bool SotesImageDecoder::is_recognized_impl(File &file) const
{
    file.stream.seek(0x438);
    auto a = file.stream.read_u32_le();
    file.stream.seek(0x448);
    auto b = file.stream.read_u32_le();
    file.stream.seek(0x450);
    auto c = file.stream.read_u32_le();
    return a - b == 0x2711 && c - b <= 0x80;
}

pix::Grid SotesImageDecoder::decode_impl(File &file) const
{
    file.stream.seek(0x448);
    auto base = file.stream.read_u32_le();

    file.stream.seek(0x450);
    auto pixel_data_offset = 0x458 + file.stream.read_u32_le() - base;

    file.stream.seek(0x430);
    auto depth = file.stream.read_u16_le() - base;

    file.stream.seek(0x440);
    file.stream.seek(4 + 4 * (file.stream.read_u32_le() - base));
    auto width = file.stream.read_u32_le() - base;

    file.stream.seek(0x18);
    file.stream.seek(0x420 + 4 * (file.stream.read_u32_le() - base));
    auto height = file.stream.read_u32_le() - base;

    file.stream.seek(0x20);
    pix::Palette palette(256, file.stream, pix::Format::BGR888X);

    file.stream.seek(pixel_data_offset);
    auto data = file.stream.read(width * height * (depth >> 3));

    std::unique_ptr<pix::Grid> pixels;
    if (depth == 8)
        pixels.reset(new pix::Grid(width, height, data, palette));
    else if (depth == 24)
        pixels.reset(new pix::Grid(width, height, data, pix::Format::BGR888));
    else
        throw err::UnsupportedBitDepthError(depth);

    pixels->flip_vertically();
    return *pixels;
}

static auto dummy = fmt::register_fmt<SotesImageDecoder>("lizsoft/sotes");
