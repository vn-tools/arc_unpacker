// XYZ image
//
// Company:   Enterbrain
// Engine:    RPGMaker
// Extension: .xyz
// Archives:  -
//
// Known games:
// - Yume Nikki

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "formats/gfx/xyz_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"
#include "string_ex.h"

static const char *xyz_magic = "XYZ1";
static const size_t xyz_magic_length = 4;

static bool xyz_decode(Converter *converter, VirtualFile *file)
{
    assert(converter != NULL);
    assert(file != NULL);

    bool result;

    char magic[xyz_magic_length];
    io_read_string(file->io, magic, xyz_magic_length);

    if (memcmp(magic, xyz_magic, xyz_magic_length) != 0)
    {
        log_error("XYZ: Not an XYZ image");
        result = false;
    }
    else
    {
        uint16_t width = io_read_u16_le(file->io);
        uint16_t height = io_read_u16_le(file->io);

        size_t compressed_data_size = io_size(file->io) - io_tell(file->io);
        char *compressed_data = (char*)malloc(compressed_data_size);
        assert(compressed_data != NULL);
        if (!io_read_string(file->io, compressed_data, compressed_data_size))
        {
            log_error("XYZ: Failed to read pixel data");
            free(compressed_data);
            return false;
        }

        char *uncompressed_data = NULL;
        size_t uncompressed_data_size = 0;
        if (!zlib_inflate(
            compressed_data,
            compressed_data_size,
            &uncompressed_data,
            &uncompressed_data_size))
        {
            log_error("XYZ: Failed to decompress zlib stream");
            free(compressed_data);
            free(uncompressed_data);
            return false;
        }
        assert(uncompressed_data != NULL);
        assert((unsigned)256 * 3 + width * height == uncompressed_data_size);
        free(compressed_data);

        size_t pixels_size = width * height * 3;
        char *pixels = (char*)malloc(pixels_size);
        assert(pixels != NULL);

        {
            char *palette = uncompressed_data;
            char *palette_indices = uncompressed_data + 256 * 3;
            char *out = pixels;

            int i;
            size_t j;
            for (i = 0; i < width * height; i ++)
            {
                j = *palette_indices ++;
                *out ++ = palette[j * 3 + 0];
                *out ++ = palette[j * 3 + 1];
                *out ++ = palette[j * 3 + 2];
            }
            free(uncompressed_data);
        }

        Image *image = image_create_from_pixels(
            width,
            height,
            pixels,
            pixels_size,
            IMAGE_PIXEL_FORMAT_RGB);
        image_update_file(image, file);
        image_destroy(image);

        free(pixels);
        result = true;
    }

    return result;
}

Converter *xyz_converter_create()
{
    Converter *converter = converter_create();
    converter->decode = &xyz_decode;
    return converter;
}
