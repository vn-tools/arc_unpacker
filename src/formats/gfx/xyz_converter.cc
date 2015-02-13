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
#include <stdexcept>
#include "formats/gfx/xyz_converter.h"
#include "formats/image.h"
#include "io.h"
#include "string_ex.h"

namespace
{
    const std::string magic("XYZ1", 4);
}

void XyzConverter::decode_internal(VirtualFile &file) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not an XYZ image");

    uint16_t width = file.io.read_u16_le();
    uint16_t height = file.io.read_u16_le();

    size_t compressed_data_size = file.io.size() - file.io.tell();
    std::string compressed_data = file.io.read(compressed_data_size);
    std::string uncompressed_data = zlib_inflate(compressed_data);
    assert(uncompressed_data.size() == (unsigned)256 * 3 + width * height);

    size_t pixels_size = width * height * 3;
    std::unique_ptr<char> pixels(new char[pixels_size]);

    {
        const char *palette = uncompressed_data.data();
        const char *palette_indices = palette + 256 * 3;
        char *out = pixels.get();

        int i;
        size_t j;
        for (i = 0; i < width * height; i ++)
        {
            j = *palette_indices ++;
            *out ++ = palette[j * 3 + 0];
            *out ++ = palette[j * 3 + 1];
            *out ++ = palette[j * 3 + 2];
        }
    }

    std::unique_ptr<Image> image = Image::from_pixels(
        width,
        height,
        std::string(pixels.get(), pixels_size),
        IMAGE_PIXEL_FORMAT_RGB);
    image->update_file(file);
}
