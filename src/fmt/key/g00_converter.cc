// G00 image
//
// Company:   Key
// Engine:    -
// Extension: .g00
// Archives:  -
//
// Known games:
// - Clannad
// - Little Busters

#include <cassert>
#include "fmt/key/g00_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::key;

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
    const u8 *src = input.get<u8>();
    const u8 *src_guardian = src + input.size();
    bstr output;
    output.resize(output_size);
    u8 *dst = output.get<u8>();
    u8 *dst_guardian = dst + output.size();

    int flag = *src++;
    int bit = 1;
    while (dst < dst_guardian)
    {
        if (bit == 256)
        {
            if (src >= src_guardian)
                break;
            flag = *src++;
            bit = 1;
        }

        if (flag & bit)
        {
            for (auto i : util::range(byte_count))
            {
                if (src >= src_guardian || dst >= dst_guardian)
                    break;
                *dst++ = *src++;
            }
        }
        else
        {
            if (src >= src_guardian)
                break;
            size_t tmp = *src++;
            if (src >= src_guardian)
                break;
            tmp |= (*src++) << 8;

            int look_behind = (tmp >> 4) * byte_count;
            size_t size = ((tmp & 0x0F) + size_delta) * byte_count;
            for (auto i : util::range(size))
            {
                if (dst >= dst_guardian)
                    break;
                assert(&dst[-look_behind] >= output.get<u8>());
                *dst = dst[-look_behind];
                dst++;
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

    auto image = util::Image::from_pixels(
        width, height, decompressed, util::PixelFormat::BGR);
    return image->create_file(file.name);
}

static std::unique_ptr<File> decode_v1(File &file, size_t width, size_t height)
{
    size_t compressed_size = file.io.read_u32_le() - 8;
    size_t decompressed_size = file.io.read_u32_le();

    auto decompressed = decompress(
        file.io.read(compressed_size), decompressed_size, 1, 2);

    io::BufferedIO tmp_io(decompressed);

    std::unique_ptr<u32[]> palette(new u32[256]);
    size_t color_count = tmp_io.read_u16_le();
    for (auto i : util::range(color_count))
        palette[i] = tmp_io.read_u32_le();

    bstr pixels;
    pixels.resize(width * height * 4);
    u32 *pixels_ptr = pixels.get<u32>();
    for (auto i : util::range(width * height))
        *pixels_ptr++ = palette[tmp_io.read_u8()];

    auto image = util::Image::from_pixels(
        width, height, pixels, util::PixelFormat::BGRA);
    return image->create_file(file.name);
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

    bstr pixels;
    pixels.resize(width * height * 4);

    io::BufferedIO decompressed_io(decompressed);
    if (region_count != decompressed_io.read_u32_le())
        throw std::runtime_error("Invalid region count");

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
        assert(block_type == 1);

        decompressed_io.skip(0x70);
        for (auto j : util::range(part_count))
        {
            u16 part_x = decompressed_io.read_u16_le();
            u16 part_y = decompressed_io.read_u16_le();
            decompressed_io.skip(2);
            u16 part_width = decompressed_io.read_u16_le();
            u16 part_height = decompressed_io.read_u16_le();
            decompressed_io.skip(0x52);

            size_t target_x = region.x1 + part_x;
            size_t target_y = region.y1 + part_y;
            assert(target_x + part_width <= width);
            assert(target_y + part_height <= height);
            for (auto y : util::range(part_height))
            {
                decompressed_io.read(
                    &pixels.get<u32>()[target_x + (target_y + y) * width],
                    part_width * 4);
            }
        }
    }

    auto image = util::Image::from_pixels(
        width, height, pixels, util::PixelFormat::BGRA);
    return image->create_file(file.name);
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
            throw std::runtime_error("Unknown G00 version");
    }
}
