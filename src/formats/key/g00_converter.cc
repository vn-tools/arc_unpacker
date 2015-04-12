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
#include "formats/key/g00_converter.h"
#include "io/buffered_io.h"
#include "util/endian.h"
#include "util/image.h"
using namespace Formats::Key;

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

    void decompress(
        const char *input,
        size_t input_size,
        char *output,
        size_t output_size,
        size_t byte_count,
        size_t length_delta)
    {
        assert(input != nullptr);
        assert(output != nullptr);

        const unsigned char *src = (unsigned char*)input;
        const unsigned char *src_guardian = src + input_size;
        unsigned char *dst = (unsigned char*)output;
        unsigned char *dst_guardian = dst + output_size;

        int flag = *src ++;
        int bit = 1;
        size_t i, look_behind, length;
        while (dst < dst_guardian)
        {
            if (bit == 256)
            {
                if (src >= src_guardian)
                    break;
                flag = *src ++;
                bit = 1;
            }

            if (flag & bit)
            {
                for (i = 0; i < byte_count; i ++)
                {
                    if (src >= src_guardian || dst >= dst_guardian)
                        break;
                    *dst ++ = *src ++;
                }
            }
            else
            {
                if (src >= src_guardian)
                    break;
                i = *src ++;
                if (src >= src_guardian)
                    break;
                i |= (*src ++) << 8;

                look_behind = (i >> 4) * byte_count;
                length = ((i & 0x0f) + length_delta) * byte_count;
                for (i = 0; i < length; i ++)
                {
                    if (dst >= dst_guardian)
                        break;
                    assert(dst >= (unsigned char*)output + look_behind);
                    *dst = dst[-(signed)look_behind];
                    dst ++;
                }
            }
            bit <<= 1;
        }
    }

    std::unique_ptr<char> decompress_from_io(
        IO &io,
        size_t compressed_size,
        size_t uncompressed_size,
        size_t byte_count,
        size_t length_delta)
    {
        std::unique_ptr<char>uncompressed(new char[uncompressed_size]);
        std::unique_ptr<char>compressed(new char[compressed_size]);

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

    std::unique_ptr<File> decode_version_0(File &file, int width, int height)
    {
        size_t compressed_size = file.io.read_u32_le();
        size_t uncompressed_size = file.io.read_u32_le();
        compressed_size -= 8;
        if (compressed_size != file.io.size() - file.io.tell())
            throw std::runtime_error("Compressed data size mismatch");

        if (uncompressed_size != (unsigned)(width * height * 4))
            throw std::runtime_error("Uncompressed data size mismatch");

        std::unique_ptr<char> uncompressed = decompress_from_io(
            file.io,
            compressed_size,
            uncompressed_size,
            3, 1);

        std::unique_ptr<Image> image = Image::from_pixels(
            width,
            height,
            std::string(uncompressed.get(), width * height * 3),
            PixelFormat::BGR);
        return image->create_file(file.name);
    }

    std::unique_ptr<File> decode_version_1(File &file, int width, int height)
    {
        size_t compressed_size = file.io.read_u32_le();
        size_t uncompressed_size = file.io.read_u32_le();
        compressed_size -= 8;
        if (compressed_size != file.io.size() - file.io.tell())
            throw std::runtime_error("Compressed data size mismatch");

        std::unique_ptr<char> uncompressed = decompress_from_io(
            file.io,
            compressed_size,
            uncompressed_size,
            1, 2);

        char *tmp = uncompressed.get();
        uint16_t color_count = le32toh(*(uint16_t*)tmp);
        tmp += 2;

        if (uncompressed_size != (unsigned)color_count * 4 + width * height + 2)
            throw std::runtime_error("Uncompressed data size mismatch");
        uint32_t *palette = (uint32_t*)tmp;
        tmp += color_count * 4;

        size_t i;
        std::unique_ptr<uint32_t> pixels(new uint32_t[width * height]);
        for (i = 0; i < (unsigned)(width * height); i ++)
        {
            unsigned char palette_index = (unsigned char)*tmp ++;
            pixels.get()[i] = palette[palette_index];
        }

        std::unique_ptr<Image> image = Image::from_pixels(
            width,
            height,
            std::string(
                reinterpret_cast<char*>(pixels.get()),
                width * height * 4),
            PixelFormat::BGRA);
        return image->create_file(file.name);
    }

    std::vector<std::unique_ptr<Region>> read_version_2_regions(
        IO &file_io, size_t region_count)
    {
        std::vector<std::unique_ptr<Region>> regions;
        regions.reserve(region_count);

        size_t i;
        for (i = 0; i < region_count; i ++)
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

    std::unique_ptr<File> decode_version_2(File &file, int width, int height)
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

        std::unique_ptr<char>uncompressed = decompress_from_io(
            file.io,
            compressed_size,
            uncompressed_size,
            1, 2);

        std::unique_ptr<uint32_t> pixels(new uint32_t[width * height]);

        BufferedIO uncompressed_io(uncompressed.get(), uncompressed_size);
        if (region_count != uncompressed_io.read_u32_le())
            throw std::runtime_error("Invalid region count");

        for (i = 0; i < region_count; i ++)
        {
            uncompressed_io.seek(4 + i * 8);
            size_t block_offset = uncompressed_io.read_u32_le();
            size_t block_size = uncompressed_io.read_u32_le();

            Region *region = regions[i].get();
            if (block_size <= 0)
                continue;

            uncompressed_io.seek(block_offset);
            uint16_t block_type = uncompressed_io.read_u16_le();
            uint16_t part_count = uncompressed_io.read_u16_le();
            assert(block_type == 1);

            uncompressed_io.skip(0x70);
            for (j = 0; j < part_count; j ++)
            {
                uint16_t part_x = uncompressed_io.read_u16_le();
                uint16_t part_y = uncompressed_io.read_u16_le();
                uncompressed_io.skip(2);
                uint16_t part_width = uncompressed_io.read_u16_le();
                uint16_t part_height = uncompressed_io.read_u16_le();
                uncompressed_io.skip(0x52);

                for (size_t y = part_y + region->y1; y < (unsigned) part_y + part_height; y ++)
                {
                    uncompressed_io.read(
                        &pixels.get()[part_x + region->x1 + y * width],
                        part_width * sizeof(uint32_t));
                }
            }
        }

        std::unique_ptr<Image> image = Image::from_pixels(
            width,
            height,
            std::string(
                reinterpret_cast<char*>(pixels.get()),
                width * height * 4),
            PixelFormat::BGRA);
        return image->create_file(file.name);
    }
}

bool G00Converter::is_recognized_internal(File &file) const
{
    return file.has_extension("g00");
}

std::unique_ptr<File> G00Converter::decode_internal(File &file) const
{
    uint8_t version = file.io.read_u8();
    uint16_t width = file.io.read_u16_le();
    uint16_t height = file.io.read_u16_le();

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
