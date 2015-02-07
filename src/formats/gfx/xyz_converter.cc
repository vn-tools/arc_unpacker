// XYZ image
//
// Company:   Enterbrain
// Engine:    RPGMaker
// Extension: .xyz
// Archives:  -
//
// Known games:
// - Yume Nikki

#include <cassert>
#include <cstring>
#include <cstring>
#include <memory>
#include "formats/gfx/xyz_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"
#include "string_ex.h"

namespace
{
    const char *xyz_magic = "XYZ1";
    const size_t xyz_magic_length = 4;
}

bool XyzConverter::decode_internal(VirtualFile &file)
{
    bool result;

    char magic[xyz_magic_length];
    file.io.read(magic, xyz_magic_length);

    if (memcmp(magic, xyz_magic, xyz_magic_length) != 0)
    {
        log_error("XYZ: Not an XYZ image");
        result = false;
    }
    else
    {
        uint16_t width = file.io.read_u16_le();
        uint16_t height = file.io.read_u16_le();

        size_t compressed_data_size = file.io.size() - file.io.tell();
        std::unique_ptr<char> compressed_data(new char[compressed_data_size]);
        assert(compressed_data != nullptr);
        file.io.read(compressed_data.get(), compressed_data_size);

        char *uncompressed_data = nullptr;
        size_t uncompressed_data_size = 0;
        if (!zlib_inflate(
            compressed_data.get(),
            compressed_data_size,
            &uncompressed_data,
            &uncompressed_data_size))
        {
            log_error("XYZ: Failed to decompress zlib stream");
            delete []uncompressed_data;
            return false;
        }
        assert(uncompressed_data != nullptr);
        assert((unsigned)256 * 3 + width * height == uncompressed_data_size);

        size_t pixels_size = width * height * 3;
        char *pixels = new char[pixels_size];
        assert(pixels != nullptr);

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
            delete []uncompressed_data;
        }

        Image *image = image_create_from_pixels(
            width,
            height,
            pixels,
            pixels_size,
            IMAGE_PIXEL_FORMAT_RGB);
        image_update_file(image, file);
        image_destroy(image);

        delete []pixels;
        result = true;
    }

    return result;
}
