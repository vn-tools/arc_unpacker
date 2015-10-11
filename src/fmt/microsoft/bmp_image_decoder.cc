#include "fmt/microsoft/bmp_image_decoder.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
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

static Header read_header(io::IO &io)
{
    auto header_size = io.read_u32_le();
    io::BufferedIO header_io(io.read(header_size - 4));

    Header h;
    h.width = header_io.read_u32_le();
    s32 height = header_io.read_u32_le();
    h.height = std::abs(height);
    h.flip = height > 0;
    h.planes = header_io.read_u16_le();
    h.depth = header_io.read_u16_le();
    h.compression = header_io.read_u32_le();
    h.image_size = header_io.read_u32_le();
    header_io.skip(8);
    h.palette_size = header_io.read_u32_le();
    h.important_colors = header_io.read_u32_le();

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
                h.masks[2] = io.read_u32_be();
                h.masks[1] = io.read_u32_be();
                h.masks[0] = io.read_u32_be();
                h.masks[3] = 0;
            }
            else if (header_size == 52)
            {
                h.masks[2] = header_io.read_u32_be();
                h.masks[1] = header_io.read_u32_be();
                h.masks[0] = header_io.read_u32_be();
                h.masks[3] = 0;
            }
            else if (header_size >= 56)
            {
                h.masks[2] = header_io.read_u32_be();
                h.masks[1] = header_io.read_u32_be();
                h.masks[0] = header_io.read_u32_be();
                h.masks[3] = header_io.read_u32_be();
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

    return h;
}

bool BmpImageDecoder::is_recognized_impl(File &file) const
{
    if (file.io.read(magic.size()) != magic)
        return false;
    return file.io.read_u32_le() == file.io.size();
}

pix::Grid BmpImageDecoder::decode_impl(File &file) const
{
    file.io.seek(10);
    auto data_offset = file.io.read_u32_le();
    auto header = read_header(file.io);
    pix::Palette palette(header.palette_size);
    for (auto i : util::range(palette.size()))
    {
        palette[i].b = file.io.read_u8();
        palette[i].g = file.io.read_u8();
        palette[i].r = file.io.read_u8();
        palette[i].a = 0xFF;
        file.io.skip(1);
    }

    if (header.planes != 1)
        throw err::NotSupportedError("Unexpected plane count");

    if (header.compression != 0 && header.compression != 3)
        throw err::NotSupportedError("Compressed BMPs are not supported");

    pix::Grid pixels(header.width, header.height);

    auto stride = ((header.depth * header.width + 31) / 32) * 4;
    if (palette.size() > 0)
    {
        for (auto y : util::range(header.height))
        {
            file.io.seek(data_offset + stride * y);
            io::BitReader bit_reader(file.io);
            for (auto x : util::range(header.width))
            {
                auto c = bit_reader.get(header.depth);
                if (c < palette.size())
                    pixels.at(x, y) = palette[c];
            }
        }
    }
    else
    {
        double multipliers[4];
        for (auto i : util::range(4))
            multipliers[i] = 255.0 / std::max<size_t>(1, header.masks[i]);

        for (auto y : util::range(header.height))
        {
            file.io.seek(data_offset + stride * y);
            io::BitReader bit_reader(file.io);
            for (auto x : util::range(header.width))
            {
                u64 c = bit_reader.get(header.depth);
                if (header.rotation < 0)
                    c = rotl(c, header.depth, -header.rotation);
                else if (header.rotation > 0)
                    c = rotr(c, header.depth, header.rotation);
                auto &p = pixels.at(x, y);
                p.b = (c & header.masks[0]) * multipliers[0];
                p.g = (c & header.masks[1]) * multipliers[1];
                p.r = (c & header.masks[2]) * multipliers[2];
                p.a = (c & header.masks[3]) * multipliers[3];
            }
        }
        if (!header.masks[3])
            for (auto &c : pixels)
                c.a = 0xFF;
    }

    if (header.flip)
        pixels.flip();
    return pixels;
}

static auto dummy = fmt::register_fmt<BmpImageDecoder>("microsoft/bmp");
