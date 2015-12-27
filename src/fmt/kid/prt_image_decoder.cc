#include "fmt/kid/prt_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "PRT\x00"_b;

bool PrtImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PrtImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto version = input_file.stream.read_u16_le();

    if (version != 0x66 && version != 0x65)
        throw err::UnsupportedVersionError(version);

    const auto bit_depth = input_file.stream.read_u16_le();
    const auto palette_offset = input_file.stream.read_u16_le();
    const auto data_offset = input_file.stream.read_u16_le();
    auto width = input_file.stream.read_u16_le();
    auto height = input_file.stream.read_u16_le();
    bool has_alpha = false;

    if (version == 0x66)
    {
        has_alpha = input_file.stream.read_u32_le() != 0;
        const auto x = input_file.stream.read_u32_le();
        const auto y = input_file.stream.read_u32_le();
        const auto width2 = input_file.stream.read_u32_le();
        const auto height2 = input_file.stream.read_u32_le();
        if (width2) width = width2;
        if (height2) height = height2;
    }

    const auto stride = (((width * bit_depth / 8) + 3) / 4) * 4;

    res::Palette palette(
        bit_depth == 8 ? 256 : 0,
        input_file.stream.seek(palette_offset),
        res::PixelFormat::BGR888X);

    res::Image image(width, height);
    for (const auto y : algo::range(height))
    {
        input_file.stream.seek(data_offset + y * stride);
        const auto row = input_file.stream.read(stride);
        const auto *row_ptr = row.get<const u8>();
        for (const auto x : algo::range(width))
        {
            if (bit_depth == 8)
            {
                image.at(x, y) = palette[*row_ptr++];
            }
            else if (bit_depth == 24)
            {
                image.at(x, y)
                    = res::read_pixel<res::PixelFormat::BGR888>(row_ptr);
            }
            else if (bit_depth == 32)
            {
                image.at(x, y)
                    = res::read_pixel<res::PixelFormat::BGRA8888>(row_ptr);
            }
            else
            {
                throw err::UnsupportedBitDepthError(bit_depth);
            }
        }
    }

    image.flip_vertically();

    if (has_alpha)
    {
        image.apply_mask(res::Image(
            width, height, input_file.stream, res::PixelFormat::Gray8));
    }

    return image;
}

static auto dummy = fmt::register_fmt<PrtImageDecoder>("kid/prt");
