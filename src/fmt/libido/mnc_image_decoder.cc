#include "fmt/libido/mnc_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::fmt::libido;

static const bstr magic = "\x48\x48\x36\x10\x0E\x00\x00\x00\x00\x00"_b;

bool MncImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid MncImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());
    auto offset_to_pixels = file.stream.read_u32_le();
    auto header_size = file.stream.read_u32_le();
    auto width = file.stream.read_u32_le();
    auto height = file.stream.read_u32_le();
    file.stream.skip(2);
    auto bit_depth = file.stream.read_u16_le();
    file.stream.skip(4);
    auto data_size = file.stream.read_u32_le();
    file.stream.seek(offset_to_pixels);
    auto data = file.stream.read(data_size);

    if (bit_depth != 24)
        throw err::UnsupportedBitDepthError(bit_depth);

    pix::Grid pixels(width, height, data, pix::Format::BGR888);
    pixels.flip_vertically();
    return pixels;
}

static auto dummy = fmt::register_fmt<MncImageDecoder>("libido/mnc");
