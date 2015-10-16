#include "fmt/truevision/tga_image_decoder.h"
#include "err.h"
#include "io/bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::truevision;

namespace
{
    enum Flags
    {
        RightToLeft = 0x10,
        TopToBottom = 0x20,
        Interleave2 = 0x40,
        Interleave4 = 0x80,
    };
}

static pix::Palette read_palette(io::IO &io, size_t size, size_t depth)
{
    if (depth == 32)
        return pix::Palette(size, io.read(size * 4), pix::Format::BGRA8888);

    if (depth == 24)
        return pix::Palette(size, io.read(size * 3), pix::Format::BGR888);

    if (depth == 16 || depth == 15)
    {
        pix::Palette palette(size);
        for (auto i : util::range(size))
        {
            palette[i] = pix::read<pix::Format::BGR555X>(io);
            palette[i].a = 0xFF;
        }
        return palette;
    }

    throw err::UnsupportedBitDepthError(depth);
}

static bstr read_compressed_pixel_data(
    io::IO &io, const size_t width, const size_t height, const size_t channels)
{
    bstr output;
    output.reserve(width * height * channels);
    while (output.size() < output.capacity())
    {
        const auto control = io.read_u8();
        const auto repetitions = (control & 0x7F) + 1;
        const bool use_rle = control & 0x80;
        if (use_rle)
        {
            const auto chunk = io.read(channels);
            for (auto i : util::range(repetitions))
                output += chunk;
        }
        else
        {
            for (auto i : util::range(repetitions))
                output += io.read(channels);
        }
    }
    return output;
}

static bstr read_uncompressed_pixel_data(
    io::IO &io, const size_t width, const size_t height, const size_t channels)
{
    return io.read(width * height * channels);
}

static pix::Grid get_pixels_from_palette(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth,
    const pix::Palette &palette)
{
    io::BitReader bit_reader(input);
    pix::Grid output(width, height);
    for (auto y : util::range(height))
    for (auto x : util::range(width))
        output.at(x, y) = palette[bit_reader.get(depth)];
    return output;
}

static pix::Grid get_pixels_without_palette(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    pix::Format format;
    if (depth == 8)
        format = pix::Format::Gray8;
    else if (depth == 16)
        format = pix::Format::BGRA5551;
    else if (depth == 24)
        format = pix::Format::BGR888;
    else if (depth == 32)
        format = pix::Format::BGRA8888;
    else
        throw err::UnsupportedBitDepthError(depth);
    return pix::Grid(width, height, input, format);
}

bool TgaImageDecoder::is_recognized_impl(File &file) const
{
    // there's no magic in the header. there is *optional* footer that *might*
    // contain the magic, but checking for this causes conflicts with certain
    // archives that contain TGA files at the end (they understandably get
    // mistaken for TGA footer).
    return file.has_extension("tga");
}

pix::Grid TgaImageDecoder::decode_impl(File &file) const
{
    file.io.seek(0);
    const auto id_size = file.io.read_u8();
    const bool use_palette = file.io.read_u8() == 1;
    const auto data_type = file.io.read_u8();
    const auto palette_start = file.io.read_u16_le();
    const auto palette_size = file.io.read_u16_le() - palette_start;
    const auto palette_depth = file.io.read_u8();
    file.io.skip(4); // x and y
    const auto width = file.io.read_u16_le();
    const auto height = file.io.read_u16_le();
    auto depth = file.io.read_u8();
    if (!depth)
        depth = 32;
    const auto flags = file.io.read_u8();

    const auto channels = depth / 8;
    const bool flip_horizontally = flags & Flags::RightToLeft;
    const bool flip_vertically = !(flags & Flags::TopToBottom);
    const bool compressed = data_type & 8;
    const size_t interleave
        = flags & Flags::Interleave2 ? 2
        : flags & Flags::Interleave4 ? 4 : 1;

    file.io.skip(id_size);

    std::unique_ptr<pix::Palette> palette;
    if (use_palette)
    {
        palette = std::make_unique<pix::Palette>(
            read_palette(file.io, palette_size, palette_depth));
    }

    const auto data = compressed
        ? read_compressed_pixel_data(file.io, width, height, channels)
        : read_uncompressed_pixel_data(file.io, width, height, channels);

    pix::Grid pixels = use_palette
        ? get_pixels_from_palette(data, width, height, depth, *palette)
        : get_pixels_without_palette(data, width, height, depth);

    if (flip_vertically)
        pixels.flip_vertically();
    if (flip_horizontally)
        pixels.flip_horizontally();
    return pixels;
}

static auto dummy = fmt::register_fmt<TgaImageDecoder>("truevision/tga");
