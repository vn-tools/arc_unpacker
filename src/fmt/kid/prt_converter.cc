// PRT image file
//
// Company:   KID
// Engine:    -
// Extension: - (inside .cps files)
// Archives:  LNK
//
// Known games:
// - Ever 17

#include <iostream>
#include "fmt/kid/prt_converter.h"
#include "util/format.h"
#include "util/image.h"
#include "util/colors.h"

using namespace au;
using namespace au::fmt::kid;

static const std::string magic = "PRT\x00"_s;

bool PrtConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PrtConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto version = file.io.read_u16_le();

    if (version != 0x66 && version != 0x65)
    {
        throw std::runtime_error(util::format(
            "Unsupported version: %d", version));
    }

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
    std::unique_ptr<u32[]> pixel_data(new u32[width * height]());

    std::unique_ptr<u32[]> palette(new u32[256]);
    if (bit_depth == 8)
    {
        file.io.seek(palette_offset);
        for (size_t i = 0; i < 256; i++)
        {
            palette[i] = util::color::rgb888(
                file.io.read_u8(), file.io.read_u8(), file.io.read_u8());
            file.io.skip(1);
        }
    }

    for (size_t y = 0; y < height; y++)
    {
        file.io.seek(data_offset + stride * y);
        u32 *out = &pixel_data[(height - 1 - y) * width];
        for (size_t x = 0; x < width; x++)
        {
            if (bit_depth == 8)
            {
                *out++ = palette[file.io.read_u8()];
            }
            else if (bit_depth == 24)
            {
                *out++ = util::color::rgb888(
                    file.io.read_u8(), file.io.read_u8(), file.io.read_u8());
            }
            else if (bit_depth == 32)
            {
                *out++ = file.io.read_u32_le();
            }
            else
            {
                throw std::runtime_error(util::format(
                    "Unsupported bit depth: %d", bit_depth));
            }
        }
    }

    if (has_alpha)
    {
        for (size_t y = 0; y < height; y++)
        for (size_t x = 0; x < width; x++)
        {
            auto alpha = file.io.read_u8();
            util::color::set_alpha(pixel_data[y * width + x], alpha);
        }
    }

    auto image = util::Image::from_pixels(
        width,
        height,
        std::string(
            reinterpret_cast<char*>(pixel_data.get()), width * height * 4),
        util::PixelFormat::BGRA);

    return image->create_file(file.name);
}
