#include "dec/microsoft/bmp_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"
#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::dec::microsoft;

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

static Header read_header(io::IStream &input_stream)
{
    Header h;
    h.data_offset = input_stream.read_u32_le();

    auto header_size = input_stream.read_u32_le();
    io::MemoryStream header_stream(input_stream.read(header_size - 4));

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
                h.masks[2] = input_stream.read_u32_be();
                h.masks[1] = input_stream.read_u32_be();
                h.masks[0] = input_stream.read_u32_be();
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
                    algo::format("Unknown header size: %d", header_size));
            }
        }
    }

    // Make the 16BPP masks sane (they're randomly rotated because why not)
    if (h.depth == 16)
    {
        for (const auto i : algo::range(4))
            h.masks[i] >>= 16;
        // detect rotation assuming red component doesn't wrap
        while (!(h.masks[2] & 0x8000))
        {
            for (const auto i : algo::range(4))
                h.masks[i] = rotl(h.masks[i], 16, 1);
            h.rotation--;
        }
    }

    h.stride = ((h.depth * h.width + 31) / 32) * 4;
    return h;
}

static res::Image get_image_from_palette(
    io::IStream &input_stream,
    const Header &header,
    const res::Palette &palette)
{
    res::Image image(header.width, header.height);
    for (const auto y : algo::range(header.height))
    {
        io::MsbBitReader bit_reader(input_stream
            .seek(header.data_offset + header.stride * y)
            .read((header.width * header.depth + 7) / 8));
        for (const auto x : algo::range(header.width))
        {
            auto c = bit_reader.get(header.depth);
            if (c < palette.size())
                image.at(x, y) = palette[c];
        }
    }
    return image;
}

static std::unique_ptr<res::Image> get_image_without_palette_fast24(
    io::IStream &input_stream, const Header &header)
{
    auto image = std::make_unique<res::Image>(header.width, header.height);
    for (const auto y : algo::range(header.height))
    {
        input_stream.seek(header.data_offset + header.stride * y);
        res::Image row(header.width, 1, input_stream, res::PixelFormat::BGR888);
        for (const auto x : algo::range(header.width))
            image->at(x, y) = row.at(x, 0);
    }
    return image;
}

static std::unique_ptr<res::Image> get_image_without_palette_fast32(
    io::IStream &input_stream, const Header &header)
{
    auto image = std::make_unique<res::Image>(header.width, header.height);
    for (const auto y : algo::range(header.height))
    {
        input_stream.seek(header.data_offset + header.stride * y);
        res::Image row(
            header.width, 1, input_stream, res::PixelFormat::BGRA8888);
        for (const auto x : algo::range(header.width))
            image->at(x, y) = row.at(x, 0);
    }
    return image;
}

static std::unique_ptr<res::Image> get_image_without_palette_generic(
    io::IStream &input_stream, const Header &header)
{
    auto image = std::make_unique<res::Image>(header.width, header.height);
    double multipliers[4];
    for (const auto i : algo::range(4))
        multipliers[i] = 255.0 / std::max<size_t>(1, header.masks[i]);

    for (const auto y : algo::range(header.height))
    {
        io::MsbBitReader bit_reader(input_stream
            .seek(header.data_offset + header.stride * y)
            .read((header.width * header.depth + 7) / 8));
        for (const auto x : algo::range(header.width))
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

static res::Image get_image_without_palette(
    io::IStream &input_stream, const Header &header)
{
    std::unique_ptr<res::Image> image;

    if (header.depth == 24
        && header.masks[0] == 0xFF0000
        && header.masks[1] == 0xFF00
        && header.masks[2] == 0xFF
        && header.masks[3] == 0)
    {
        image = get_image_without_palette_fast24(input_stream, header);
    }

    else if (header.depth == 32
        && header.masks[0] == 0xFF000000
        && header.masks[1] == 0xFF0000
        && header.masks[2] == 0xFF00
        && (header.masks[3] == 0 || header.masks[3] == 0xFF))
    {
        image = get_image_without_palette_fast32(input_stream, header);
    }

    else
    {
        image = get_image_without_palette_generic(input_stream, header);
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

res::Image BmpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(10);
    auto header = read_header(input_file.stream);
    res::Palette palette(header.palette_size);
    for (const auto i : algo::range(palette.size()))
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

    res::Image image = palette.size() > 0
        ? get_image_from_palette(input_file.stream, header, palette)
        : get_image_without_palette(input_file.stream, header);

    if (header.flip)
        image.flip_vertically();
    return image;
}

static auto _ = dec::register_decoder<BmpImageDecoder>("microsoft/bmp");
