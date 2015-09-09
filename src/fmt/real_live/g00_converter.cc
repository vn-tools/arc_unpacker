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
    struct Region
    {
        size_t x1;
        size_t y1;
        size_t x2;
        size_t y2;
        size_t ox;
        size_t oy;
    };
}

static bstr decompress(
    const bstr &input,
    size_t output_size,
    size_t byte_count,
    size_t size_delta)
{
    const u8 *src_ptr = input.get<u8>();
    const u8 *src_end = src_ptr + input.size();
    bstr output;
    output.resize(output_size);
    auto dst_ptr = output.get<u8>();
    auto dst_start = output.get<const u8>();
    auto dst_end = output.end<const u8>();

    int flag = *src_ptr++;
    int bit = 1;
    while (dst_ptr < dst_end)
    {
        if (bit == 256)
        {
            if (src_ptr >= src_end)
                break;
            flag = *src_ptr++;
            bit = 1;
        }

        if (flag & bit)
        {
            for (auto i : util::range(byte_count))
            {
                if (src_ptr >= src_end || dst_ptr >= dst_end)
                    break;
                *dst_ptr++ = *src_ptr++;
            }
        }
        else
        {
            if (src_ptr >= src_end)
                break;
            size_t tmp = *src_ptr++;
            if (src_ptr >= src_end)
                break;
            tmp |= (*src_ptr++) << 8;

            int look_behind = (tmp >> 4) * byte_count;
            size_t size = ((tmp & 0x0F) + size_delta) * byte_count;
            while (size-- && dst_ptr < dst_end)
            {
                if (&dst_ptr[-look_behind] < dst_start)
                    break;
                *dst_ptr = dst_ptr[-look_behind];
                dst_ptr++;
            }
        }
        bit <<= 1;
    }

    return output;
}

static std::unique_ptr<File> decode_v0(File &file, size_t width, size_t height)
{
    size_t compressed_size = file.io.read_u32_le() - 8;
    size_t decompressed_size = file.io.read_u32_le();

    auto decompressed = decompress(
        file.io.read(compressed_size), decompressed_size, 3, 1);

    pix::Grid pixels(width, height, decompressed, pix::Format::BGR888);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static std::unique_ptr<File> decode_v1(File &file, size_t width, size_t height)
{
    size_t compressed_size = file.io.read_u32_le() - 8;
    size_t decompressed_size = file.io.read_u32_le();

    auto decompressed = decompress(
        file.io.read(compressed_size), decompressed_size, 1, 2);
    io::BufferedIO tmp_io(decompressed);

    size_t colors = tmp_io.read_u16_le();
    auto pal_data = tmp_io.read(4 * colors);
    auto pix_data = tmp_io.read_to_eof();

    pix::Palette palette(colors, pal_data, pix::Format::BGRA8888);
    pix::Grid pixels(width, height, pix_data, palette);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static std::vector<std::unique_ptr<Region>> read_v2_regions(
    io::IO &file_io, size_t region_count)
{
    std::vector<std::unique_ptr<Region>> regions;
    regions.reserve(region_count);

    for (auto i : util::range(region_count))
    {
        std::unique_ptr<Region> region(new Region);
        region->x1 = file_io.read_u32_le();
        region->y1 = file_io.read_u32_le();
        region->x2 = file_io.read_u32_le();
        region->y2 = file_io.read_u32_le();
        region->ox = file_io.read_u32_le();
        region->oy = file_io.read_u32_le();
        regions.push_back(std::move(region));
    }
    return regions;
}

static std::unique_ptr<File> decode_v2(File &file, size_t width, size_t height)
{
    size_t region_count = file.io.read_u32_le();
    auto regions = read_v2_regions(file.io, region_count);

    size_t compressed_size = file.io.read_u32_le() - 8;
    size_t decompressed_size = file.io.read_u32_le();

    auto decompressed = decompress(
        file.io.read(compressed_size), decompressed_size, 1, 2);

    pix::Grid pixels(width, height);

    io::BufferedIO decompressed_io(decompressed);
    if (region_count != decompressed_io.read_u32_le())
        throw err::CorruptDataError("Region count mismatch");

    for (auto i : util::range(region_count))
    {
        decompressed_io.seek(4 + i * 8);
        size_t block_offset = decompressed_io.read_u32_le();
        size_t block_size = decompressed_io.read_u32_le();

        Region &region = *regions[i];
        if (block_size <= 0)
            continue;

        decompressed_io.seek(block_offset);
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

            size_t target_x = region.x1 + part_x;
            size_t target_y = region.y1 + part_y;
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
