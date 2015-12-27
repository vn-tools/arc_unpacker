#include "fmt/libido/mnc_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::fmt::libido;

static const bstr magic = "\x48\x48\x36\x10\x0E\x00\x00\x00\x00\x00"_b;

bool MncImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image MncImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto offset_to_pixels = input_file.stream.read_u32_le();
    auto header_size = input_file.stream.read_u32_le();
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    input_file.stream.skip(2);
    auto bit_depth = input_file.stream.read_u16_le();
    input_file.stream.skip(4);
    auto data_size = input_file.stream.read_u32_le();
    input_file.stream.seek(offset_to_pixels);
    auto data = input_file.stream.read(data_size);

    if (bit_depth != 24)
        throw err::UnsupportedBitDepthError(bit_depth);

    res::Image image(width, height, data, res::PixelFormat::BGR888);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<MncImageDecoder>("libido/mnc");
