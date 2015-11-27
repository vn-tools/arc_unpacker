#include "fmt/playstation/gim_image_decoder.h"
#include <map>
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::playstation;

static const bstr magic = "MIG.00.1PSP\x00"_b;

namespace
{
    struct Chunk final
    {
        int type;
        size_t offset, size;
    };
}

static std::unique_ptr<pix::Palette> read_palette(
    io::Stream &stream, const Chunk &chunk)
{
    stream.seek(chunk.offset + 0x14);
    const auto format_id = stream.read_u16_le();
    stream.skip(2);
    const auto color_count = stream.read_u16_le();

    pix::Format format;
    if      (format_id == 0) format = pix::Format::RGB565;
    else if (format_id == 1) format = pix::Format::RGBA5551;
    else if (format_id == 2) format = pix::Format::RGBA4444;
    else if (format_id == 3) format = pix::Format::RGBA8888;
    else throw err::NotSupportedError("Unknown palette format");

    stream.seek(chunk.offset + 0x50);
    return std::make_unique<pix::Palette>(color_count, stream, format);
}

static bstr read_data(
    io::Stream &stream,
    const size_t width,
    const size_t height,
    const size_t bpp,
    const bool swizzled)
{
    const auto stride = width * bpp / 8;
    const auto block_stride = stride / 16;
    auto data = stream.read(height * stride);
    if (swizzled)
    {
        const bstr source = data;
        for (const auto y : util::range(height))
        for (const auto x : util::range(stride))
        {
            const auto block_x = x / 16;
            const auto block_y = y / 8;
            const auto block_idx = block_x + block_y * block_stride;
            const auto block_offset = block_idx * 16 * 8;
            const auto idx = block_offset
                + (x - block_x * 16)
                + ((y - block_y * 8) * 16);
            data[x + y * stride] = source.at(idx);
        }
    }
    return data;
}

static std::unique_ptr<pix::Image> read_image(
    io::Stream &stream,
    const Chunk &chunk,
    std::unique_ptr<pix::Palette> palette)
{
    stream.seek(chunk.offset + 0x14);
    const auto format_id = stream.read_u16_le();
    const auto swizzled = stream.read_u16_le() == 0x01;
    const auto width = (stream.read_u16_le() + 15) & ~15;
    const auto height = (stream.read_u16_le() + 7) & ~7;

    stream.seek(chunk.offset + 0x2C);
    const auto data_offset = stream.read_u32_le();
    stream.seek(chunk.offset + 0x10 + data_offset);
    std::unique_ptr<pix::Image> image;
    if (format_id < 4)
    {
        pix::Format format;

        if      (format_id == 0) format = pix::Format::RGB565;
        else if (format_id == 1) format = pix::Format::RGBA5551;
        else if (format_id == 2) format = pix::Format::RGBA4444;
        else if (format_id == 3) format = pix::Format::RGBA8888;

        const auto data = read_data(
            stream, width, height, pix::format_to_bpp(format) * 8, swizzled);
        image = std::make_unique<pix::Image>(width, height, data, format);
    }
    else
    {
        size_t palette_bits;
        if      (format_id == 4) palette_bits = 4;
        else if (format_id == 5) palette_bits = 8;
        else if (format_id == 6) palette_bits = 16;
        else if (format_id == 7) palette_bits = 32;
        else throw err::NotSupportedError("Unknown pixel format");

        const auto data = read_data(
            stream, width, height, palette_bits, swizzled);
        if (palette_bits == 8)
            image = std::make_unique<pix::Image>(width, height, data, *palette);
        else if (palette_bits == 4)
        {
            io::MemoryStream data_stream(data);
            image = std::make_unique<pix::Image>(width, height);
            if (palette_bits == 4)
            {
                for (const auto y : util::range(height))
                for (const auto x : util::range(0, width, 2))
                {
                    const auto tmp = data_stream.read_u8();
                    image->at(x + 0, y) = palette->at(tmp & 0xF);
                    image->at(x + 1, y) = palette->at(tmp >> 4);
                }
            }
            else if (palette_bits == 16)
            {
                for (const auto y : util::range(height))
                for (const auto x : util::range(width))
                    image->at(x, y) = palette->at(data_stream.read_u16_le());
            }
            else if (palette_bits == 32)
            {
                for (const auto y : util::range(height))
                for (const auto x : util::range(width))
                    image->at(x, y) = palette->at(data_stream.read_u32_le());
            }
        }
    }

    return image;
}

bool GimImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Image GimImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(0x30);
    std::map<int, Chunk> chunks;
    while (!input_file.stream.eof())
    {
        Chunk chunk;
        chunk.offset = input_file.stream.tell();
        chunk.type = input_file.stream.read_u32_le();
        chunk.size = input_file.stream.read_u32_le();
        if (input_file.stream.read_u32_le() != chunk.size)
            throw err::NotSupportedError("Data is most probably compressed");
        input_file.stream.seek(chunk.offset + chunk.size);
        chunks[chunk.type] = chunk;
    }

    std::unique_ptr<pix::Palette> palette;
    if (chunks.find(0x05) != chunks.end())
        palette = read_palette(input_file.stream, chunks[0x05]);

    if (chunks.find(0x04) == chunks.end())
        throw err::CorruptDataError("Missing bitmap");
    return *read_image(input_file.stream, chunks[0x04], std::move(palette));
}

static auto dummy = fmt::register_fmt<GimImageDecoder>("playstation/gim");
