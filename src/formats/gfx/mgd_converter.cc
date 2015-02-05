// MGD image
//
// Company:   -
// Engine:    -
// Extension: .MGD
// Archives:  FJSYS

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "collections/array.h"
#include "endian.h"
#include "formats/gfx/mgd_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"

static const char *mgd_magic = "MGD ";
static const size_t mgd_magic_length = 4;

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

static bool mgd_check_magic(IO *io)
{
    assert(io != NULL);
    char magic[mgd_magic_length];
    io_read_string(io, magic, mgd_magic_length);
    return memcmp(magic, mgd_magic, mgd_magic_length) == 0;
}

static bool mgd_decompress_sgd_alpha(
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

static bool mgd_decompress_sgd_bgr(
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
        switch (flag & 0xc0)
        {
            case 0x80:
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
                break;
            }

            case 0x40:
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
                break;
            }

            case 0:
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
                break;
            }

            default:
            {
                log_error("SGD: Bad decompression flag");
                return false;
            }
        }
    }
    *input = input_ptr;
    *output = output_ptr;
    return true;
}

static bool mgd_decompress_sgd(
    const char *const input,
    size_t input_size,
    char *const output,
    size_t output_size)
{
    assert(input != NULL);
    assert(output != NULL);

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

static Array *mgd_read_region_data(IO *file_io)
{
    Array *regions = array_create();
    while (io_tell(file_io) < io_size(file_io))
    {
        io_skip(file_io, 4);
        size_t regions_size = io_read_u32_le(file_io);
        size_t region_count = io_read_u16_le(file_io);
        size_t meta_format = io_read_u16_le(file_io);
        size_t bytes_left = io_size(file_io) - io_tell(file_io);
        if (meta_format != 4)
        {
            log_warning("MGD: Unexpected region format %d", meta_format);
            array_destroy(regions);
            return NULL;
        }
        if (regions_size != bytes_left)
        {
            log_warning("MGD: Unexpected region size %d", regions_size);
            array_destroy(regions);
            return NULL;
        }

        size_t i;
        for (i = 0; i < region_count; i ++)
        {
            MgdRegion *region = (MgdRegion*)malloc(sizeof(MgdRegion));
            if (!region)
            {
                log_error("MGD: Failed to allocate memory for region");
                continue;
            }
            region->x = io_read_u16_le(file_io);
            region->y = io_read_u16_le(file_io);
            region->width = io_read_u16_le(file_io);
            region->height = io_read_u16_le(file_io);
            array_add(regions, region);
        }

        io_skip(file_io, 4);
    }
    return regions;
}

static Image *mgd_read_image(
    IO *file_io,
    MgdCompressionType compression_type,
    size_t size_compressed,
    size_t size_original,
    size_t image_width,
    size_t image_height,
    char **data_uncompressed)
{
    assert(file_io != NULL);
    assert(data_uncompressed != NULL);

    char *data_compressed = (char*)malloc(size_compressed);
    if (!data_compressed)
    {
        log_error("MGD: Failed to allocate memory for compressed data");
        return NULL;
    }
    if (!io_read_string(file_io, data_compressed, size_compressed))
    {
        log_error("MGD: Failed to read compressed data");
        return NULL;
    }

    log_info("MGD: compression type = %d", compression_type);

    Image *image = NULL;
    switch (compression_type)
    {
        case MGD_COMPRESSION_NONE:
            *data_uncompressed = data_compressed;
            image = image_create_from_pixels(
                image_width,
                image_height,
                data_compressed,
                size_compressed,
                IMAGE_PIXEL_FORMAT_BGRA);
            break;

        case MGD_COMPRESSION_SGD:
        {
            *data_uncompressed = (char*)malloc(size_original);
            assert(*data_uncompressed != NULL);

            if (mgd_decompress_sgd(
                data_compressed,
                size_compressed,
                *data_uncompressed,
                size_original))
            {
                image = image_create_from_pixels(
                    image_width,
                    image_height,
                    *data_uncompressed,
                    size_original,
                    IMAGE_PIXEL_FORMAT_BGRA);
            }
            else
            {
                log_error("MGD: Failed to decompress SGD data");
            }

            free(data_compressed);
            break;
        }

        case MGD_COMPRESSION_PNG:
        {
            IO *image_io = io_create_empty();
            io_write_string(
                image_io,
                data_compressed,
                size_compressed);
            image = image_create_from_boxed(image_io);
            io_destroy(image_io);
            free(data_compressed);
            break;
        }
    }

    return image;
}

static bool mgd_decode(Converter *converter, VirtualFile *file)
{
    assert(converter != NULL);
    assert(file != NULL);

    if (!mgd_check_magic(file->io))
    {
        log_error("MGD: Not a MGD graphic file");
        return false;
    }

    __attribute__((unused)) uint16_t data_offset = io_read_u16_le(file->io);
    __attribute__((unused)) uint16_t format = io_read_u16_le(file->io);
    io_skip(file->io, 4);
    uint16_t image_width = io_read_u16_le(file->io);
    uint16_t image_height = io_read_u16_le(file->io);
    uint32_t size_original = io_read_u32_le(file->io);
    uint32_t size_compressed_total = io_read_u32_le(file->io);
    MgdCompressionType compression_type
        = (MgdCompressionType)io_read_u32_le(file->io);
    io_skip(file->io, 64);

    size_t size_compressed = io_read_u32_le(file->io);
    if (size_compressed + 4 != size_compressed_total)
    {
        log_error(
            "MGD: Compressed data size verification failed (%d != %d)",
            size_compressed,
            size_compressed_total);
        return false;
    }

    char *data_uncompressed = NULL;
    Image *image = mgd_read_image(
        file->io,
        compression_type,
        size_compressed,
        size_original,
        image_width,
        image_height,
        &data_uncompressed);

    Array *regions = mgd_read_region_data(file->io);
    size_t i;
    for (i = 0; i < array_size(regions); i ++)
        free(array_get(regions, i));
    array_destroy(regions);

    if (image == NULL)
    {
        log_error("MGD: No image produced");
        return false;
    }

    image_update_file(image, file);
    image_destroy(image);
    free(data_uncompressed);
    return true;
}

Converter *mgd_converter_create()
{
    Converter *converter = converter_create();
    converter->decode = &mgd_decode;
    return converter;
}
