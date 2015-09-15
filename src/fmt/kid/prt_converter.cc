// PRT image file
//
// Company:   KID
// Engine:    -
// Extension: - (inside .cps files)
// Archives:  LNK
//
// Known games:
// - [KID] [031127] Ever 17

#include "fmt/kid/prt_converter.h"
#include "err.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "PRT\x00"_b;

bool PrtConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PrtConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto version = file.io.read_u16_le();

    if (version != 0x66 && version != 0x65)
        throw err::UnsupportedVersionError(version);

    u16 bit_depth = file.io.read_u16_le();
    u16 palette_offset = file.io.read_u16_le();
    u16 data_offset = file.io.read_u16_le();
    u32 width = file.io.read_u16_le();
    u32 height = file.io.read_u16_le();
    bool has_alpha = false;

    if (version == 0x66)
    {
        has_alpha = file.io.read_u32_le();
        auto x = file.io.read_u32_le();
        auto y = file.io.read_u32_le();
        auto width2 = file.io.read_u32_le();
        auto height2 = file.io.read_u32_le();
        if (width2) width = width2;
        if (height2) height = height2;
    }

    auto stride = (((width * bit_depth / 8) + 3) / 4) * 4;

    file.io.seek(palette_offset);
    pix::Palette palette(
        bit_depth == 8 ? 256 : 0, file.io, pix::Format::BGR888X);

    pix::Grid pixels(width, height);
    for (auto y : util::range(height))
    {
        file.io.seek(data_offset + y * stride);
        auto row = file.io.read(stride);
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

    pixels.flip();

    if (has_alpha)
    {
        for (auto y : util::range(height))
        for (auto x : util::range(width))
            pixels.at(x, y).a = file.io.read_u8();
    }

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<PrtConverter>("kid/prt");
