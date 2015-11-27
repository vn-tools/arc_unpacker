#include "fmt/lizsoft/sotes_image_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lizsoft;

bool SotesImageDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto a = input_file.stream.seek(0x438).read_u32_le();
    const auto b = input_file.stream.seek(0x448).read_u32_le();
    const auto c = input_file.stream.seek(0x450).read_u32_le();
    return a - b == 0x2711 && c - b <= 0x80;
}

res::Image SotesImageDecoder::decode_impl(io::File &input_file) const
{
    const auto base = input_file.stream.seek(0x448).read_u32_le();
    const auto pixel_data_offset
        = 0x458 + input_file.stream.seek(0x450).read_u32_le() - base;

    const auto depth = input_file.stream.seek(0x430).read_u16_le() - base;
    auto tmp1 = input_file.stream.seek(0x440).read_u32_le() - base;
    const auto width
        = input_file.stream.seek(4 + 4 * tmp1).read_u32_le() - base;

    const auto tmp2 = input_file.stream.seek(0x18).read_u32_le() - base;
    const auto height
        = input_file.stream.seek(0x420 + 4 * tmp2).read_u32_le() - base;

    res::Palette palette(
        256,
        input_file.stream
            .seek(0x20)
            .read(256 * 4),
        res::PixelFormat::BGR888X);

    const auto data = input_file.stream
        .seek(pixel_data_offset)
        .read(width * height * (depth >> 3));

    std::unique_ptr<res::Image> image;
    if (depth == 8)
    {
        image = std::make_unique<res::Image>(width, height, data, palette);
    }
    else if (depth == 24)
    {
        image = std::make_unique<res::Image>(
            width, height, data, res::PixelFormat::BGR888);
    }
    else
    {
        throw err::UnsupportedBitDepthError(depth);
    }

    image->flip_vertically();
    return *image;
}

static auto dummy = fmt::register_fmt<SotesImageDecoder>("lizsoft/sotes");
