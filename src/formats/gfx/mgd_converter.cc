// MGD image
//
// Company:   -
// Engine:    -
// Extension: .MGD
// Archives:  FJSYS

#include <cassert>
#include <cstdint>
#include <cstring>
#include "buffered_io.h"
#include "endian.h"
#include "formats/gfx/mgd_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"

namespace
{
    const char *mgd_magic = "MGD ";
    const size_t mgd_magic_length = 4;

    typedef enum
    {
        MGD_COMPRESSION_NONE = 0,
        MGD_COMPRESSION_SGD = 1,
        MGD_COMPRESSION_PNG = 2,
    } MgdCompressionType;

    typedef struct
    {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
    } MgdRegion;

    bool mgd_check_magic(IO &io)
    {
        char magic[mgd_magic_length];
        io.read(magic, mgd_magic_length);
        return memcmp(magic, mgd_magic, mgd_magic_length) == 0;
    }

    bool mgd_decompress_sgd_alpha(
        const uint8_t **input,
        const uint8_t *const input_guardian,
        uint8_t **output,
        const uint8_t *const output_guardian)
    {
        const uint8_t *input_ptr = *input;
        uint8_t *output_ptr = *output + 3;
        while (input_ptr < input_guardian)
        {
            uint16_t flag = le16toh(*(uint16_t*)input_ptr);
            input_ptr += 2;
            if (flag & 0x8000)
            {
                size_t pixels = (flag & 0x7fff) + 1;
                uint8_t alpha = *input_ptr ++;
                size_t i;
                for (i = 0; i < pixels; i ++)
                {
                    if (output_ptr > output_guardian)
                    {
                        log_warning("SGD: trying to write alpha beyond EOF");
                        return false;
                    }
                    *output_ptr = alpha ^ 0xff;
                    output_ptr += 4;
                }
            }

            else
            {
                while (flag -- && input_ptr < input_guardian)
                {
                    uint8_t alpha = *input_ptr;
                    input_ptr ++;
                    if (output_ptr > output_guardian)
                    {
                        log_warning("SGD: trying to write alpha beyond EOF");
                        return false;
                    }
                    *output_ptr = alpha ^ 0xff;
                    output_ptr += 4;
                }
            }
        }
        *input = input_ptr;
        *output = output_ptr;
        return true;
    }

    bool mgd_decompress_sgd_bgr(
        const uint8_t **input,
        const uint8_t *const input_guardian,
        uint8_t **output,
        const uint8_t *const output_guardian)
    {
        const uint8_t *input_ptr = *input;
        uint8_t *output_ptr = *output;

        while (input_ptr < input_guardian)
        {
            uint8_t flag = *input_ptr ++;
            size_t i;

            if ((flag & 0xc0) == 0x80)
            {
                size_t pixels = flag & 0x3f;
                uint8_t b = output_ptr[-4];
                uint8_t g = output_ptr[-3];
                uint8_t r = output_ptr[-2];
                for (i = 0; i < pixels; i ++)
                {
                    if (input_ptr + 2 > input_guardian)
                    {
                        log_warning("SGD: trying to read length beyond EOF");
                        return false;
                    }
                    uint16_t delta = le16toh(*(uint16_t*)input_ptr);
                    input_ptr += 2;

                    if (delta & 0x8000)
                    {
                        b += delta & 0x1f;
                        g += (delta >> 5) & 0x1f;
                        r += (delta >> 10) & 0x1f;
                    }
                    else
                    {
                        b += ( delta        & 0xf) * (delta &   0x10 ? -1 : 1);
                        g += ((delta >>  5) & 0xf) * (delta &  0x200 ? -1 : 1);
                        r += ((delta >> 10) & 0xf) * (delta & 0x4000 ? -1 : 1);
                    }

                    if (output_ptr + 4 > output_guardian)
                    {
                        log_warning("SGD: trying to write colors beyond EOF");
                        return false;
                    }
                    *output_ptr ++ = b;
                    *output_ptr ++ = g;
                    *output_ptr ++ = r;
                    output_ptr ++;
                }
            }

            else if ((flag & 0xc0) == 0x40)
            {
                if (input_ptr + 3 > input_guardian)
                {
                    log_warning("SGD: trying to read colors beyond EOF");
                    return false;
                }
                size_t pixels = (flag & 0x3f) + 1;
                uint8_t b = *input_ptr ++;
                uint8_t g = *input_ptr ++;
                uint8_t r = *input_ptr ++;
                for (i = 0; i < pixels; i ++)
                {
                    if (output_ptr + 4 > output_guardian)
                    {
                        log_warning("SGD: trying to write colors beyond EOF");
                        return false;
                    }
                    *output_ptr ++ = b;
                    *output_ptr ++ = g;
                    *output_ptr ++ = r;
                    output_ptr ++;
                }
            }

            else if ((flag & 0xc0) == 0)
            {
                size_t pixels = flag;
                for (i = 0; i < pixels; i ++)
                {
                    if (input_ptr + 3 > input_guardian)
                    {
                        log_warning("SGD: trying to read colors beyond EOF");
                        return false;
                    }
                    if (output_ptr + 4 > output_guardian)
                    {
                        log_warning("SGD: trying to write colors beyond EOF");
                        return false;
                    }
                    *output_ptr ++ = *input_ptr ++;
                    *output_ptr ++ = *input_ptr ++;
                    *output_ptr ++ = *input_ptr ++;
                    output_ptr ++;
                }
            }

            else
            {
                log_error("SGD: Bad decompression flag");
                return false;
            }
        }
        *input = input_ptr;
        *output = output_ptr;
        return true;
    }

    bool mgd_decompress_sgd(
        const char *const input,
        size_t input_size,
        char *const output,
        size_t output_size)
    {
        assert(input != nullptr);
        assert(output != nullptr);

        size_t length;
        const uint8_t *input_guardian;
        const uint8_t *output_guardian = (const uint8_t*)output + output_size;
        uint8_t *output_ptr = (uint8_t*)output;

        const uint8_t *input_ptr = (const uint8_t*)input;
        length = le32toh(*(uint32_t*)input_ptr);
        input_ptr += 4;
        input_guardian = (const uint8_t*)input_ptr + length;
        if (length > input_size)
        {
            log_error("SGD: insufficient alpha channel data");
            return false;
        }

        if (!mgd_decompress_sgd_alpha(
            &input_ptr,
            input_guardian,
            &output_ptr,
            output_guardian))
        {
            log_warning("SGD: failed to decompress alpha channel");
        }

        length = le32toh(*(uint32_t*)input_ptr);
        input_ptr += 4;
        input_guardian = (const uint8_t*)input_ptr + length;
        if (length > input_size)
        {
            log_error("SGD: insufficient color data");
            return false;
        }

        output_ptr = (uint8_t*)output;
        if (!mgd_decompress_sgd_bgr(
            &input_ptr,
            input_guardian,
            &output_ptr,
            output_guardian))
        {
            log_warning("SGD: failed to decompress color channels");
        }

        return true;
    }

    std::vector<std::unique_ptr<MgdRegion>> mgd_read_region_data(IO &file_io)
    {
        std::vector<std::unique_ptr<MgdRegion>> regions;
        while (file_io.tell() < file_io.size())
        {
            file_io.skip(4);
            size_t regions_size = file_io.read_u32_le();
            size_t region_count = file_io.read_u16_le();
            size_t meta_format = file_io.read_u16_le();
            size_t bytes_left = file_io.size() - file_io.tell();
            if (meta_format != 4)
                throw std::runtime_error("Unexpected region format");
            if (regions_size != bytes_left)
                throw std::runtime_error("Region size mismatch");

            for (size_t i = 0; i < region_count; i ++)
            {
                std::unique_ptr<MgdRegion> region(new MgdRegion);
                if (!region)
                {
                    log_error("MGD: Failed to allocate memory for region");
                    continue;
                }
                region->x = file_io.read_u16_le();
                region->y = file_io.read_u16_le();
                region->width = file_io.read_u16_le();
                region->height = file_io.read_u16_le();
                regions.push_back(std::move(region));
            }

            if (file_io.tell() + 4 >= file_io.size())
                break;
            file_io.skip(4);
        }
        return regions;
    }

    std::unique_ptr<Image> mgd_read_image(
        IO &file_io,
        MgdCompressionType compression_type,
        size_t size_compressed,
        size_t size_original,
        size_t image_width,
        size_t image_height)
    {
        std::string data_compressed = file_io.read(size_compressed);
        log_info("MGD: compression type = %d", compression_type);

        switch (compression_type)
        {
            case MGD_COMPRESSION_NONE:
                return Image::from_pixels(
                    image_width,
                    image_height,
                    data_compressed,
                    IMAGE_PIXEL_FORMAT_BGRA);

            case MGD_COMPRESSION_SGD:
            {
                std::unique_ptr<char> data_uncompressed(
                    new char[size_original]);

                mgd_decompress_sgd(
                    data_compressed.data(),
                    size_compressed,
                    data_uncompressed.get(),
                    size_original);

                return Image::from_pixels(
                    image_width,
                    image_height,
                    std::string(data_uncompressed.get(), size_original),
                    IMAGE_PIXEL_FORMAT_BGRA);
            }

            case MGD_COMPRESSION_PNG:
            {
                BufferedIO buffered_io(data_compressed);
                return Image::from_boxed(buffered_io);
            }

            default:
                throw std::runtime_error("Unsupported compression type");
        }
    }
}

void MgdConverter::decode_internal(VirtualFile &file) const
{
    if (!mgd_check_magic(file.io))
        throw std::runtime_error("MGD: Not a MGD graphic file");

    __attribute__((unused)) uint16_t data_offset = file.io.read_u16_le();
    __attribute__((unused)) uint16_t format = file.io.read_u16_le();
    file.io.skip(4);
    uint16_t image_width = file.io.read_u16_le();
    uint16_t image_height = file.io.read_u16_le();
    uint32_t size_original = file.io.read_u32_le();
    uint32_t size_compressed_total = file.io.read_u32_le();
    MgdCompressionType compression_type
        = (MgdCompressionType)file.io.read_u32_le();
    file.io.skip(64);

    size_t size_compressed = file.io.read_u32_le();
    if (size_compressed + 4 != size_compressed_total)
        throw std::runtime_error("Compressed data size mismatch");

    std::unique_ptr<Image> image = mgd_read_image(
        file.io,
        compression_type,
        size_compressed,
        size_original,
        image_width,
        image_height);

    __attribute__((unused)) auto regions = mgd_read_region_data(file.io);

    image->update_file(file);
}
