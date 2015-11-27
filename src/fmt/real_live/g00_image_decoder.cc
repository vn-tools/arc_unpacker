#include "fmt/real_live/g00_image_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::real_live;

namespace
{
    struct Region final
    {
        size_t x1, y1;
        size_t x2, y2;
        size_t ox, oy;
        size_t block_offset, block_size;
    };
}

static bstr decompress(
    const bstr &input, size_t output_size, size_t byte_count, size_t size_delta)
{
    bstr output(output_size);

    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output.end<const u8>();
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input.end<const u8>();

    u16 control = 1;
    while (output_ptr < output_end && input_ptr < input_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = *input_ptr++ | 0xFF00;

        if (control & 1)
        {
            auto size = byte_count;
            while (size-- && output_ptr < output_end && input_ptr < input_end)
                *output_ptr++ = *input_ptr++;
        }
        else
        {
            u16 tmp = *input_ptr++;
            if (input_ptr >= input_end)
                break;
            tmp |= *input_ptr++ << 8;

            int look_behind = (tmp >> 4) * byte_count;
            size_t size = ((tmp & 0x0F) + size_delta) * byte_count;
            u8 *source_ptr = &output_ptr[-look_behind];
            if (source_ptr < output.get<const u8>())
                break;
            while (size-- && output_ptr < output_end && source_ptr < output_end)
                *output_ptr++ = *source_ptr++;
        }
    }
    return output;
}

static pix::Image decode_v0(io::File &input_file, size_t width, size_t height)
{
    const auto size_comp = input_file.stream.read_u32_le() - 8;
    const auto size_orig = input_file.stream.read_u32_le();
    const auto data = decompress(
        input_file.stream.read(size_comp), size_orig, 3, 1);
    return pix::Image(width, height, data, pix::PixelFormat::BGR888);
}

static pix::Image decode_v1(io::File &input_file, size_t width, size_t height)
{
    const auto size_comp = input_file.stream.read_u32_le() - 8;
    const auto size_orig = input_file.stream.read_u32_le();
    io::MemoryStream tmp_stream(
        decompress(input_file.stream.read(size_comp), size_orig, 1, 2));
    const size_t colors = tmp_stream.read_u16_le();
    const auto pal_data = tmp_stream.read(4 * colors);
    const auto pix_data = tmp_stream.read_to_eof();
    pix::Palette palette(colors, pal_data, pix::PixelFormat::BGRA8888);
    return pix::Image(width, height, pix_data, palette);
}

static pix::Image decode_v2(io::File &input_file, size_t width, size_t height)
{
    std::vector<std::unique_ptr<Region>> regions;
    const auto region_count = input_file.stream.read_u32_le();
    for (const auto i : util::range(region_count))
    {
        auto region = std::make_unique<Region>();
        region->x1 = input_file.stream.read_u32_le();
        region->y1 = input_file.stream.read_u32_le();
        region->x2 = input_file.stream.read_u32_le();
        region->y2 = input_file.stream.read_u32_le();
        region->ox = input_file.stream.read_u32_le();
        region->oy = input_file.stream.read_u32_le();
        regions.push_back(std::move(region));
    }

    const auto size_comp = input_file.stream.read_u32_le() - 8;
    const auto size_orig = input_file.stream.read_u32_le();

    io::MemoryStream input(
        decompress(input_file.stream.read(size_comp), size_orig, 1, 2));
    if (input.read_u32_le() != regions.size())
        throw err::CorruptDataError("Region count mismatch");

    pix::Image image(width, height);
    for (const auto &region : regions)
    {
        region->block_offset = input.read_u32_le();
        region->block_size = input.read_u32_le();
    }

    for (const auto &region : regions)
    {
        if (region->block_size <= 0)
            continue;

        input.seek(region->block_offset);
        u16 block_type = input.read_u16_le();
        u16 part_count = input.read_u16_le();
        if (block_type != 1)
            throw err::NotSupportedError("Unexpected block type");

        input.skip(0x70);
        for (const auto j : util::range(part_count))
        {
            u16 part_x = input.read_u16_le();
            u16 part_y = input.read_u16_le();
            input.skip(2);
            u16 part_width = input.read_u16_le();
            u16 part_height = input.read_u16_le();
            input.skip(0x52);

            pix::Image part(
                part_width,
                part_height,
                input,
                pix::PixelFormat::BGRA8888);

            const size_t target_x = region->x1 + part_x;
            const size_t target_y = region->y1 + part_y;
            if (target_x + part_width > width
                || target_y + part_height > height)
            {
                throw err::CorruptDataError("Region out of bounds");
            }
            for (const auto y : util::range(part_height))
            for (const auto x : util::range(part_width))
            {
                image.at(target_x + x, target_y + y) = part.at(x, y);
            }
        }
    }

    return image;
}

bool G00ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("g00");
}

pix::Image G00ImageDecoder::decode_impl(io::File &input_file) const
{
    u8 version = input_file.stream.read_u8();
    u16 width = input_file.stream.read_u16_le();
    u16 height = input_file.stream.read_u16_le();

    switch (version)
    {
        case 0:
            return decode_v0(input_file, width, height);

        case 1:
            return decode_v1(input_file, width, height);

        case 2:
            return decode_v2(input_file, width, height);
    }

    throw err::UnsupportedVersionError(version);
}

static auto dummy = fmt::register_fmt<G00ImageDecoder>("real-live/g00");
