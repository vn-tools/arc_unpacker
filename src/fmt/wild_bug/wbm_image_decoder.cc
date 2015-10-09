#include "fmt/wild_bug/wbm_image_decoder.h"
#include "err.h"
#include "fmt/wild_bug/wpx/decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1A""BMP\x00"_b;

bool WbmImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

static size_t get_stride(size_t width, size_t channels)
{
    return (width * channels + 3) & (~3);
}

static std::array<size_t, 8> get_offsets(size_t channels, size_t stride)
{
    return
    {
        channels,
        channels * 2,
        channels * 3,
        5 * channels >= stride ? channels * 4 : stride - channels,
        5 * channels >= stride ? channels * 5 : stride,
        5 * channels >= stride ? channels * 6 : stride + channels,
        5 * channels >= stride ? channels * 7 : stride + 2 * channels,
        5 * channels >= stride ? channels * 8 : 2 * stride
    };
}

static void remove_pad(
    bstr &data, size_t height, size_t out_stride, size_t in_stride)
{
    for (auto y : util::range(height))
    {
        auto output = data.get<u8>() + y * out_stride;
        auto input = data.get<u8>() + y * in_stride;
        auto size = out_stride;
        while (size-- && output < data.end<u8>())
            *output++ = *input++;
    }
}

static pix::Grid get_pixels(
    wpx::Decoder &decoder,
    u8 section_id,
    size_t width,
    size_t height,
    size_t channels)
{
    auto stride = get_stride(width, channels);
    auto offsets = get_offsets(channels, stride);
    auto data = decoder.read_compressed_section(section_id, channels, offsets);
    remove_pad(data, height, width * channels, stride);
    if (channels == 1)
        return pix::Grid(width, height, data, pix::Format::Gray8);
    else if (channels == 3)
        return pix::Grid(width, height, data, pix::Format::BGR888);
    else if (channels == 4)
        return pix::Grid(width, height, data, pix::Format::BGRA8888);
    else
        throw err::UnsupportedChannelCountError(channels);
}

pix::Grid WbmImageDecoder::decode_impl(File &file) const
{
    wpx::Decoder decoder(file.io);

    io::BufferedIO metadata_io(decoder.read_plain_section(0x10));
    metadata_io.skip(4);
    auto width = metadata_io.read_u16_le();
    auto height = metadata_io.read_u16_le();
    metadata_io.skip(4);
    auto depth = metadata_io.read_u8();

    if (depth != 32 && depth != 24 && depth != 8)
        throw err::UnsupportedBitDepthError(depth);

    auto pixels = get_pixels(decoder, 0x11, width, height, depth >> 3);
    if (decoder.has_section(0x13))
    {
        auto mask = get_pixels(decoder, 0x13, width, height, 1);
        pixels.apply_alpha_from_mask(mask);
    }

    return pixels;
}

static auto dummy = fmt::register_fmt<WbmImageDecoder>("wild-bug/wbm");
