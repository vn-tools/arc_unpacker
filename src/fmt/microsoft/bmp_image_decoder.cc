#include "fmt/microsoft/bmp_image_decoder.h"
#include "io/bit_reader.h"
#include "io/memory_stream.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::microsoft;

static const bstr magic = "BM"_b;

namespace
{
    struct Header final
    {
        size_t data_offset;
        u32 width;
        u32 height;
        u16 planes;
        u16 depth;
        u32 compression;
        u32 image_size;
        u32 palette_size;
        u32 important_colors; // what
        u32 masks[4]; // BGRA
        int rotation;
        bool flip;
        size_t stride;
    };
}

static u64 rotl(u64 value, size_t value_size, size_t how_much)
{
    how_much %= value_size;
    auto ret = (value << how_much) | (value >> (value_size - how_much));
    return ret & ((1ull << value_size) - 1);
}

static u64 rotr(u64 value, size_t value_size, size_t how_much)
{
    return rotl(value, value_size, value_size - how_much);
}

static Header read_header(io::Stream &stream)
{
    Header h;
    h.data_offset = stream.read_u32_le();

    auto header_size = stream.read_u32_le();
    io::MemoryStream header_stream(stream.read(header_size - 4));

    h.width = header_stream.read_u32_le();
    s32 height = header_stream.read_u32_le();
    h.height = std::abs(height);
    h.flip = height > 0;
    h.planes = header_stream.read_u16_le();
    h.depth = header_stream.read_u16_le();
    h.compression = header_stream.read_u32_le();
    h.image_size = header_stream.read_u32_le();
    header_stream.skip(8);
    h.palette_size = header_stream.read_u32_le();
    h.important_colors = header_stream.read_u32_le();

    if (h.depth <= 8 && !h.palette_size)
        h.palette_size = 256;
    if (h.depth > 8)
        h.palette_size = 0;

    h.rotation = 0;
    if (!h.palette_size)
    {
        if (h.compression != 3)
        {
            if (h.depth == 24)
            {
                h.masks[2] = 0x0000FF;
                h.masks[1] = 0x00FF00;
                h.masks[0] = 0xFF0000;
                h.masks[3] = 0;
            }
            else if (h.depth == 32)
            {
                h.masks[2] = 0x0000FF00;
                h.masks[1] = 0x00FF0000;
                h.masks[0] = 0xFF000000;
                h.masks[3] = 0;
            }
            else if (h.depth == 16)
            {
                h.masks[2] = 0b0000000001111100'0000000000000000;
                h.masks[1] = 0b1110000000000011'0000000000000000;
                h.masks[0] = 0b0001111100000000'0000000000000000;
                h.masks[3] = 0;
            }
            else
                throw err::UnsupportedBitDepthError(h.depth);
        }

        else
        {
            if (header_size == 40)
            {
                h.masks[2] = stream.read_u32_be();
                h.masks[1] = stream.read_u32_be();
                h.masks[0] = stream.read_u32_be();
                h.masks[3] = 0;
            }
            else if (header_size == 52)
            {
                h.masks[2] = header_stream.read_u32_be();
                h.masks[1] = header_stream.read_u32_be();
                h.masks[0] = header_stream.read_u32_be();
                h.masks[3] = 0;
            }
            else if (header_size >= 56)
            {
                h.masks[2] = header_stream.read_u32_be();
                h.masks[1] = header_stream.read_u32_be();
                h.masks[0] = header_stream.read_u32_be();
                h.masks[3] = header_stream.read_u32_be();
            }
            else
            {
                throw err::NotSupportedError(
                    util::format("Unknown header size: %d", header_size));
            }
        }
    }

    // Make the 16BPP masks sane (they're randomly rotated because why not)
    if (h.depth == 16)
    {
        for (auto i : util::range(4))
            h.masks[i] >>= 16;
        // detect rotation assuming red component doesn't wrap
        while (!(h.masks[2] & 0x8000))
        {
            for (auto i : util::range(4))
                h.masks[i] = rotl(h.masks[i], 16, 1);
            h.rotation--;
        }
    }

    h.stride = ((h.depth * h.width + 31) / 32) * 4;
    return h;
}

static pix::Image get_image_from_palette(
    io::Stream &stream,
    const Header &header,
    const pix::Palette &palette)
{
    pix::Image image(header.width, header.height);
    for (auto y : util::range(header.height))
    {
        stream.seek(header.data_offset + header.stride * y);
        io::BitReader bit_reader(stream);
        for (auto x : util::range(header.width))
        {
            auto c = bit_reader.get(header.depth);
            if (c < palette.size())
                image.at(x, y) = palette[c];
        }
    }
    return image;
}

static std::unique_ptr<pix::Image> get_image_without_palette_fast24(
    io::Stream &stream, const Header &header)
{
    auto image = std::make_unique<pix::Image>(header.width, header.height);
    for (auto y : util::range(header.height))
    {
        stream.seek(header.data_offset + header.stride * y);
        pix::Image row(header.width, 1, stream, pix::PixelFormat::BGR888);
        for (auto x : util::range(header.width))
            image->at(x, y) = row.at(x, 0);
    }
    return image;
}

static std::unique_ptr<pix::Image> get_image_without_palette_fast32(
    io::Stream &stream, const Header &header)
{
    auto image = std::make_unique<pix::Image>(header.width, header.height);
    for (auto y : util::range(header.height))
    {
        stream.seek(header.data_offset + header.stride * y);
        pix::Image row(header.width, 1, stream, pix::PixelFormat::BGRA8888);
        for (auto x : util::range(header.width))
            image->at(x, y) = row.at(x, 0);
    }
    return image;
}

static std::unique_ptr<pix::Image> get_image_without_palette_generic(
    io::Stream &stream, const Header &header)
{
    auto image = std::make_unique<pix::Image>(header.width, header.height);
    double multipliers[4];
    for (auto i : util::range(4))
        multipliers[i] = 255.0 / std::max<size_t>(1, header.masks[i]);

    for (auto y : util::range(header.height))
    {
        stream.seek(header.data_offset + header.stride * y);
        io::BitReader bit_reader(stream);
        for (auto x : util::range(header.width))
        {
            u64 c = bit_reader.get(header.depth);
            if (header.rotation < 0)
                c = rotl(c, header.depth, -header.rotation);
            else if (header.rotation > 0)
                c = rotr(c, header.depth, header.rotation);
            auto &p = image->at(x, y);
            p.b = (c & header.masks[0]) * multipliers[0];
            p.g = (c & header.masks[1]) * multipliers[1];
            p.r = (c & header.masks[2]) * multipliers[2];
            p.a = (c & header.masks[3]) * multipliers[3];
        }
    }
    return image;
}

static pix::Image get_image_without_palette(
    io::Stream &stream, const Header &header)
{
    std::unique_ptr<pix::Image> image;

    if (header.depth == 24
        && header.masks[0] == 0xFF0000
        && header.masks[1] == 0xFF00
        && header.masks[2] == 0xFF
        && header.masks[3] == 0)
    {
        image = get_image_without_palette_fast24(stream, header);
    }

    else if (header.depth == 32
        && header.masks[0] == 0xFF000000
        && header.masks[1] == 0xFF0000
        && header.masks[2] == 0xFF00
        && (header.masks[3] == 0 || header.masks[3] == 0xFF))
    {
        image = get_image_without_palette_fast32(stream, header);
    }

    else
    {
        image = get_image_without_palette_generic(stream, header);
    }

    if (!header.masks[3])
        for (auto &c : *image)
            c.a = 0xFF;

    return *image;
}

bool BmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    input_file.stream.skip(4); // file size, some encoders corrupt this value
    return input_file.stream.read_u32_le() == 0; // but this should be reliable
}

pix::Image BmpImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(10);
    auto header = read_header(input_file.stream);
    pix::Palette palette(header.palette_size);
    for (auto i : util::range(palette.size()))
    {
        palette[i].b = input_file.stream.read_u8();
        palette[i].g = input_file.stream.read_u8();
        palette[i].r = input_file.stream.read_u8();
        palette[i].a = 0xFF;
        input_file.stream.skip(1);
    }

    if (header.planes != 1)
        throw err::NotSupportedError("Unexpected plane count");

    if (header.compression != 0 && header.compression != 3)
        throw err::NotSupportedError("Compressed BMPs are not supported");

    pix::Image image = palette.size() > 0
        ? get_image_from_palette(input_file.stream, header, palette)
        : get_image_without_palette(input_file.stream, header);

    if (header.flip)
        image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<BmpImageDecoder>("microsoft/bmp");
