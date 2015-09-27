#include "fmt/libido/mnc_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::fmt::libido;

static const bstr magic = "\x48\x48\x36\x10\x0E\x00\x00\x00\x00\x00"_b;

bool MncImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid MncImageDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto offset_to_pixels = file.io.read_u32_le();
    auto header_size = file.io.read_u32_le();
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    file.io.skip(2);
    auto bit_depth = file.io.read_u16_le();
    file.io.skip(4);
    auto data_size = file.io.read_u32_le();
    file.io.seek(offset_to_pixels);
    auto data = file.io.read(data_size);

    if (bit_depth != 24)
        throw err::UnsupportedBitDepthError(bit_depth);

    pix::Grid pixels(width, height, data, pix::Format::BGR888);
    pixels.flip();
    return pixels;
}

static auto dummy = fmt::Registry::add<MncImageDecoder>("libido/mnc");
