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
#include "util/endian.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::key;

namespace
{
    typedef struct
    {
        size_t x1;
        size_t y1;
        size_t x2;
        size_t y2;
        size_t ox;
        size_t oy;
    } Region;
}

static void decompress(
    const char *input,
    size_t input_size,
    char *output,
    size_t output_size,
    size_t byte_count,
    size_t length_delta)
{
    assert(input != nullptr);
    assert(output != nullptr);

    const u8 *src = reinterpret_cast<const u8*>(input);
    const u8 *src_guardian = src + input_size;
    u8 *dst = reinterpret_cast<u8*>(output);
    u8 *dst_guardian = dst + output_size;

    int flag = *src++;
    int bit = 1;
    size_t i, length;
    int look_behind;
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
            for (i = 0; i < byte_count; i++)
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
            i = *src++;
            if (src >= src_guardian)
                break;
            i |= (*src++) << 8;

            look_behind = (i >> 4) * byte_count;
            length = ((i & 0x0F) + length_delta) * byte_count;
            for (i = 0; i < length; i++)
            {
                if (dst >= dst_guardian)
                    break;
                assert(dst >= reinterpret_cast<u8*>(output + look_behind));
                *dst = dst[-look_behind];
                dst++;
            }
        }
        bit <<= 1;
    }
}

static std::unique_ptr<char[]> decompress_from_io(
    io::IO &io,
    size_t compressed_size,
    size_t uncompressed_size,
    size_t byte_count,
    size_t length_delta)
{
    std::unique_ptr<char[]> uncompressed(new char[uncompressed_size]);
    std::unique_ptr<char[]> compressed(new char[compressed_size]);

    io.read(compressed.get(), compressed_size);

    decompress(
        compressed.get(),
        compressed_size,
        uncompressed.get(),
        uncompressed_size,
        byte_count,
        length_delta);

    return std::move(uncompressed);
}

static std::unique_ptr<File> decode_version_0(File &file, int width, int height)
{
    size_t compressed_size = file.io.read_u32_le();
    size_t uncompressed_size = file.io.read_u32_le();
    compressed_size -= 8;
    if (compressed_size != file.io.size() - file.io.tell())
        throw std::runtime_error("Compressed data size mismatch");

    if (uncompressed_size != static_cast<size_t>(width * height * 4))
        throw std::runtime_error("Uncompressed data size mismatch");

    std::unique_ptr<char[]> uncompressed = decompress_from_io(
        file.io, compressed_size, uncompressed_size, 3, 1);

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        width,
        height,
        std::string(uncompressed.get(), width * height * 3),
        util::PixelFormat::BGR);
    return image->create_file(file.name);
}

static std::unique_ptr<File> decode_version_1(File &file, int width, int height)
{
    size_t compressed_size = file.io.read_u32_le() - 8;
    size_t uncompressed_size = file.io.read_u32_le();

    std::unique_ptr<char[]> uncompressed = decompress_from_io(
        file.io, compressed_size, uncompressed_size, 1, 2);

    io::BufferedIO tmp_io(uncompressed.get(), uncompressed_size);

    std::unique_ptr<u32[]> palette(new u32[256]);
    size_t color_count = tmp_io.read_u16_le();
    for (size_t i = 0; i < color_count; i++)
        palette[i] = tmp_io.read_u32_le();

    std::unique_ptr<u32[]> pixels(new u32[width * height]);
    for (size_t i = 0; i < static_cast<size_t>(width * height); i++)
        pixels[i] = palette[tmp_io.read_u8()];

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        width,
        height,
        std::string(
            reinterpret_cast<char*>(pixels.get()),
            width * height * 4),
        util::PixelFormat::BGRA);
    return image->create_file(file.name);
}

static std::vector<std::unique_ptr<Region>> read_version_2_regions(
    io::IO &file_io, size_t region_count)
{
    std::vector<std::unique_ptr<Region>> regions;
    regions.reserve(region_count);

    size_t i;
    for (i = 0; i < region_count; i++)
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

static std::unique_ptr<File> decode_version_2(File &file, int width, int height)
{
    size_t region_count = file.io.read_u32_le();
    size_t i, j;
    std::vector<std::unique_ptr<Region>> regions
        = read_version_2_regions(file.io, region_count);

    size_t compressed_size = file.io.read_u32_le();
    size_t uncompressed_size = file.io.read_u32_le();
    compressed_size -= 8;
    if (compressed_size != file.io.size() - file.io.tell())
        throw std::runtime_error("Compressed data size mismatch");

    std::unique_ptr<char[]>uncompressed = decompress_from_io(
        file.io, compressed_size, uncompressed_size, 1, 2);

    std::unique_ptr<u32[]> pixels(new u32[width * height]);
    for (int i = 0; i < width * height; i++)
        pixels[i] = 0;

    io::BufferedIO uncompressed_io(uncompressed.get(), uncompressed_size);
    if (region_count != uncompressed_io.read_u32_le())
        throw std::runtime_error("Invalid region count");

    for (i = 0; i < region_count; i++)
    {
        uncompressed_io.seek(4 + i * 8);
        size_t block_offset = uncompressed_io.read_u32_le();
        size_t block_size = uncompressed_io.read_u32_le();

        Region *region = regions[i].get();
        if (block_size <= 0)
            continue;

        uncompressed_io.seek(block_offset);
        u16 block_type = uncompressed_io.read_u16_le();
        u16 part_count = uncompressed_io.read_u16_le();
        assert(block_type == 1);

        uncompressed_io.skip(0x70);
        for (j = 0; j < part_count; j++)
        {
            u16 part_x = uncompressed_io.read_u16_le();
            u16 part_y = uncompressed_io.read_u16_le();
            uncompressed_io.skip(2);
            u16 part_width = uncompressed_io.read_u16_le();
            u16 part_height = uncompressed_io.read_u16_le();
            uncompressed_io.skip(0x52);

            for (size_t y = part_y + region->y1;
                y < static_cast<size_t>(part_y + part_height);
                y++)
            {
                uncompressed_io.read(
                    &pixels[part_x + region->x1 + y * width],
                    part_width * sizeof(u32));
            }
        }
    }

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        width,
        height,
        std::string(
            reinterpret_cast<char*>(pixels.get()),
            width * height * 4),
        util::PixelFormat::BGRA);
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
            return decode_version_0(file, width, height);
            break;

        case 1:
            return decode_version_1(file, width, height);
            break;

        case 2:
            return decode_version_2(file, width, height);
            break;

        default:
            throw std::runtime_error("Unknown G00 version");
    }
}
