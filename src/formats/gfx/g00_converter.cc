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
#include <cstdio>
#include "endian.h"
#include "formats/gfx/g00_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"

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
    } G00Region;

    void g00_decompress(
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
                    *dst = dst[-look_behind];
                    dst ++;
                }
            }
            bit <<= 1;
        }
    }

    bool g00_decompress_from_io(
        IO *io,
        size_t compressed_size,
        char **uncompressed,
        size_t uncompressed_size,
        size_t byte_count,
        size_t length_delta)
    {
        assert(io != nullptr);
        assert(uncompressed != nullptr);

        *uncompressed = new char[uncompressed_size];
        if (!*uncompressed)
        {
            log_error("G00: Failed to allocate %d bytes", uncompressed_size);
            return false;
        }

        char *compressed = new char[compressed_size];
        if (!compressed)
        {
            *uncompressed = nullptr;
            delete []uncompressed;
            log_error("G00: Failed to allocate %d bytes", compressed);
            return false;
        }

        if (!io_read_string(io, compressed, compressed_size))
        {
            *uncompressed = nullptr;
            delete []compressed;
            delete []uncompressed;
            log_error("G00: Failed to read compressed stream");
            return false;
        }

        g00_decompress(
            compressed,
            compressed_size,
            *uncompressed,
            uncompressed_size,
            byte_count,
            length_delta);

        delete []compressed;
        return true;
    }

    bool g00_decode_version_0(VirtualFile *file, int width, int height)
    {
        size_t compressed_size = io_read_u32_le(file->io);
        size_t uncompressed_size = io_read_u32_le(file->io);
        compressed_size -= 8;
        if (compressed_size != io_size(file->io) - io_tell(file->io))
        {
            log_error("G00: Bad compressed size");
            return false;
        }
        if (uncompressed_size != (unsigned)(width * height * 4))
        {
            log_error("G00: Bad uncompressed size");
            return false;
        }

        char *uncompressed = nullptr;
        if (!g00_decompress_from_io(
            file->io,
            compressed_size,
            &uncompressed,
            uncompressed_size,
            3, 1))
        {
            log_error("G00: Failed to decompress data");
            delete []uncompressed;
            return false;
        }
        assert(uncompressed != nullptr);

        Image *image = image_create_from_pixels(
            width,
            height,
            uncompressed,
            width * height * 3,
            IMAGE_PIXEL_FORMAT_BGR);
        image_update_file(image, file);
        image_destroy(image);
        delete []uncompressed;
        return true;
    }

    bool g00_decode_version_1(VirtualFile *file, int width, int height)
    {
        size_t compressed_size = io_read_u32_le(file->io);
        size_t uncompressed_size = io_read_u32_le(file->io);
        compressed_size -= 8;
        if (compressed_size != io_size(file->io) - io_tell(file->io))
        {
            log_error("G00: Bad compressed size");
            return false;
        }

        char *uncompressed = nullptr;
        if (!g00_decompress_from_io(
            file->io,
            compressed_size,
            &uncompressed,
            uncompressed_size,
            1, 2))
        {
            log_error("G00: Failed to decompress data");
            delete []uncompressed;
            return false;
        }
        assert(uncompressed != nullptr);

        char *tmp = uncompressed;
        uint16_t color_count = le32toh(*(uint16_t*)tmp);
        tmp += 2;

        bool result;
        if (uncompressed_size != (unsigned)color_count * 4 + width * height + 2)
        {
            log_error("G00: Bad uncompressed size");
            result = false;
        }
        else
        {
            uint32_t *palette = (uint32_t*)tmp;
            tmp += color_count * 4;

            size_t i;
            uint32_t *pixels = new uint32_t[width * height];
            assert(pixels != nullptr);
            for (i = 0; i < (unsigned)(width * height); i ++)
            {
                unsigned char palette_index = (unsigned char)*tmp ++;
                pixels[i] = palette[palette_index];
            }

            Image *image = image_create_from_pixels(
                width,
                height,
                (char*)pixels,
                width * height * 4,
                IMAGE_PIXEL_FORMAT_BGRA);
            image_update_file(image, file);
            image_destroy(image);

            delete []pixels;
            result = true;
        }

        delete []uncompressed;
        return result;
    }

    G00Region *g00_read_version_2_regions(IO *file_io, size_t region_count)
    {
        G00Region *regions = new G00Region[region_count];
        if (!regions)
        {
            log_error(
                "G00: Failed to allocate memory for %d regions", region_count);
            return nullptr;
        }

        size_t i;
        for (i = 0; i < region_count; i ++)
        {
            regions[i].x1 = io_read_u32_le(file_io);
            regions[i].y1 = io_read_u32_le(file_io);
            regions[i].x2 = io_read_u32_le(file_io);
            regions[i].y2 = io_read_u32_le(file_io);
            regions[i].ox = io_read_u32_le(file_io);
            regions[i].oy = io_read_u32_le(file_io);
        }
        return regions;
    }

    bool g00_decode_version_2(VirtualFile *file, int width, int height)
    {
        assert(file != nullptr);

        size_t region_count = io_read_u32_le(file->io);
        size_t i, j;
        G00Region *regions = g00_read_version_2_regions(file->io, region_count);
        if (!regions)
        {
            log_error("G00: Failed to read region data");
            return false;
        }

        size_t compressed_size = io_read_u32_le(file->io);
        size_t uncompressed_size = io_read_u32_le(file->io);
        compressed_size -= 8;
        if (compressed_size != io_size(file->io) - io_tell(file->io))
        {
            log_error("G00: Bad compressed size");
            delete []regions;
            return false;
        }

        char *uncompressed = nullptr;
        if (!g00_decompress_from_io(
            file->io,
            compressed_size,
            &uncompressed,
            uncompressed_size,
            1, 2))
        {
            log_error("G00: Failed to decompress data");
            delete []uncompressed;
            delete []regions;
            return false;
        }

        char *pixels = new char[width * height * 4];
        if (!pixels)
        {
            log_error(
                "G00: Failed to allocate memory for %d x %d pixels",
                width,
                height);
            delete []uncompressed;
            delete []regions;
            return false;
        }

        bool result;
        IO *uncompressed_io = io_create_from_buffer(
            uncompressed, uncompressed_size);
        if (region_count != io_read_u32_le(uncompressed_io))
        {
            log_error("G00: Bad region count");
            result = false;
        }
        else
        {
            for (i = 0; i < region_count; i ++)
            {
                io_seek(uncompressed_io, 4 + i * 8);
                size_t block_offset = io_read_u32_le(uncompressed_io);
                size_t block_size = io_read_u32_le(uncompressed_io);

                G00Region *region = &regions[i];
                if (block_size <= 0)
                    continue;

                io_seek(uncompressed_io, block_offset);
                uint16_t block_type = io_read_u16_le(uncompressed_io);
                uint16_t part_count = io_read_u16_le(uncompressed_io);
                assert(1 == block_type);

                io_skip(uncompressed_io, 0x70);
                for (j = 0; j < part_count; j ++)
                {
                    uint16_t part_x = io_read_u16_le(uncompressed_io);
                    uint16_t part_y = io_read_u16_le(uncompressed_io);
                    io_skip(uncompressed_io, 2);
                    uint16_t part_width = io_read_u16_le(uncompressed_io);
                    uint16_t part_height = io_read_u16_le(uncompressed_io);
                    io_skip(uncompressed_io, 0x52);

                    size_t y;
                    for (y = part_y+region->y1; y < part_y+part_height; y ++)
                    {
                        if (!io_read_string(
                            uncompressed_io,
                            pixels + (part_x + region->x1 + y * width) * 4,
                            part_width * 4))
                        {
                            assert(0);
                        }
                    }
                }
            }
            result = true;
        }

        io_destroy(uncompressed_io);
        delete []uncompressed;

        Image *image = image_create_from_pixels(
            width,
            height,
            (char*)pixels,
            width * height * 4,
            IMAGE_PIXEL_FORMAT_BGRA);
        image_update_file(image, file);
        image_destroy(image);

        delete []pixels;
        delete []regions;
        return result;
    }
}

bool G00Converter::decode_internal(VirtualFile *file)
{
    assert(file != nullptr);

    uint8_t version = io_read_u8(file->io);
    uint16_t width = io_read_u16_le(file->io);
    uint16_t height = io_read_u16_le(file->io);
    log_info("G00: Version = %d", version);

    switch (version)
    {
        case 0:
            return g00_decode_version_0(file, width, height);

        case 1:
            return g00_decode_version_1(file, width, height);

        case 2:
            return g00_decode_version_2(file, width, height);

        default:
            log_error("G00: Not a G00 image");
            return false;
    }

    return true;
}
