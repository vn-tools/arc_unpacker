#include "fmt/kid/prt_image_decoder.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "PRT\x00"_b;

bool PrtImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid PrtImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());
    auto version = file.stream.read_u16_le();

    if (version != 0x66 && version != 0x65)
        throw err::UnsupportedVersionError(version);

    u16 bit_depth = file.stream.read_u16_le();
    u16 palette_offset = file.stream.read_u16_le();
    u16 data_offset = file.stream.read_u16_le();
    u32 width = file.stream.read_u16_le();
    u32 height = file.stream.read_u16_le();
    bool has_alpha = false;

    if (version == 0x66)
    {
        has_alpha = file.stream.read_u32_le();
        auto x = file.stream.read_u32_le();
        auto y = file.stream.read_u32_le();
        auto width2 = file.stream.read_u32_le();
        auto height2 = file.stream.read_u32_le();
        if (width2) width = width2;
        if (height2) height = height2;
    }

    auto stride = (((width * bit_depth / 8) + 3) / 4) * 4;

    file.stream.seek(palette_offset);
    pix::Palette palette(
        bit_depth == 8 ? 256 : 0, file.stream, pix::Format::BGR888X);

    pix::Grid pixels(width, height);
    for (auto y : util::range(height))
    {
        file.stream.seek(data_offset + y * stride);
        auto row = file.stream.read(stride);
        auto row_ptr = row.get<const u8>();
        for (auto x : util::range(width))
        {
            if (bit_depth == 8)
                pixels.at(x, y) = palette[*row_ptr++];
            else if (bit_depth == 24)
                pixels.at(x, y) = pix::read<pix::Format::BGR888>(row_ptr);
            else if (bit_depth == 32)
                pixels.at(x, y) = pix::read<pix::Format::BGRA8888>(row_ptr);
            else
                throw err::UnsupportedBitDepthError(bit_depth);
        }
    }

    pixels.flip_vertically();

    if (has_alpha)
    {
        for (auto y : util::range(height))
        for (auto x : util::range(width))
            pixels.at(x, y).a = file.stream.read_u8();
    }

    return pixels;
}

static auto dummy = fmt::register_fmt<PrtImageDecoder>("kid/prt");
