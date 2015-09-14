// G00 image
//
// Company:   -
// Engine:    RealLive
// Extension: .g00
// Archives:  -
//
// Known games:
// - [Hamham Soft] [071221] Imouto ni! Sukumizu Kisetara Nugasanai!
// - [Key] [041126] Kanon
// - [Key] [070928] Little Busters!
// - [Key] [080229] Clannad

#include "err.h"
#include "fmt/real_live/g00_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
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
    bstr output;
    output.resize(output_size);

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

static std::unique_ptr<File> decode_v0(File &file, size_t width, size_t height)
{
    size_t size_comp = file.io.read_u32_le() - 8;
    size_t size_orig = file.io.read_u32_le();

    auto decompressed = decompress(
        file.io.read(size_comp), size_orig, 3, 1);

    pix::Grid pixels(width, height, decompressed, pix::Format::BGR888);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static std::unique_ptr<File> decode_v1(File &file, size_t width, size_t height)
{
    size_t size_comp = file.io.read_u32_le() - 8;
    size_t size_orig = file.io.read_u32_le();

    auto decompressed = decompress(file.io.read(size_comp), size_orig, 1, 2);
    io::BufferedIO tmp_io(decompressed);

    size_t colors = tmp_io.read_u16_le();
    auto pal_data = tmp_io.read(4 * colors);
    auto pix_data = tmp_io.read_to_eof();

    pix::Palette palette(colors, pal_data, pix::Format::BGRA8888);
    pix::Grid pixels(width, height, pix_data, palette);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static std::unique_ptr<File> decode_v2(File &file, size_t width, size_t height)
{
    std::vector<std::unique_ptr<Region>> regions;
    regions.resize(file.io.read_u32_le());
    for (auto i : util::range(regions.size()))
    {
        std::unique_ptr<Region> region(new Region);
        region->x1 = file.io.read_u32_le();
        region->y1 = file.io.read_u32_le();
        region->x2 = file.io.read_u32_le();
        region->y2 = file.io.read_u32_le();
        region->ox = file.io.read_u32_le();
        region->oy = file.io.read_u32_le();
        regions[i] = std::move(region);
    }

    size_t size_comp = file.io.read_u32_le() - 8;
    size_t size_orig = file.io.read_u32_le();

    auto decompressed = decompress(file.io.read(size_comp), size_orig, 1, 2);
    io::BufferedIO decompressed_io(decompressed);
    if (decompressed_io.read_u32_le() != regions.size())
        throw err::CorruptDataError("Region count mismatch");

    pix::Grid pixels(width, height);
    for (auto &region : regions)
    {
        region->block_offset = decompressed_io.read_u32_le();
        region->block_size = decompressed_io.read_u32_le();
    }

    for (auto &region : regions)
    {
        if (region->block_size <= 0)
            continue;

        decompressed_io.seek(region->block_offset);
        u16 block_type = decompressed_io.read_u16_le();
        u16 part_count = decompressed_io.read_u16_le();
        if (block_type != 1)
            throw err::NotSupportedError("Unexpected block type");

        decompressed_io.skip(0x70);
        for (auto j : util::range(part_count))
        {
            u16 part_x = decompressed_io.read_u16_le();
            u16 part_y = decompressed_io.read_u16_le();
            decompressed_io.skip(2);
            u16 part_width = decompressed_io.read_u16_le();
            u16 part_height = decompressed_io.read_u16_le();
            decompressed_io.skip(0x52);

            pix::Grid part(
                part_width,
                part_height,
                decompressed_io,
                pix::Format::BGRA8888);

            size_t target_x = region->x1 + part_x;
            size_t target_y = region->y1 + part_y;
            if (target_x + part_width > width
                || target_y + part_height > height)
            {
                throw err::CorruptDataError("Region out of bounds");
            }
            for (auto y : util::range(part_height))
            for (auto x : util::range(part_width))
            {
                pixels.at(target_x + x, target_y + y) = part.at(x, y);
            }
        }
    }

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

bool G00Converter::is_recognized_internal(File &file) const
{
    return file.has_extension("g00");
}

std::unique_ptr<File> G00Converter::decode_internal(File &file) const
{
    u8 version = file.io.read_u8();
    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();

    switch (version)
    {
        case 0:
            return decode_v0(file, width, height);

        case 1:
            return decode_v1(file, width, height);

        case 2:
            return decode_v2(file, width, height);

        default:
            throw err::UnsupportedVersionError(version);
    }
}

static auto dummy = fmt::Registry::add<G00Converter>("rl/g00");
