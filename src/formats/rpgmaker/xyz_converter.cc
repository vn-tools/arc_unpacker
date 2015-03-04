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
#include "formats/rpgmaker/xyz_converter.h"
#include "formats/image.h"
#include "io.h"
#include "string/zlib.h"
using namespace Formats::RpgMaker;

namespace
{
    const std::string magic("XYZ1", 4);
}

void XyzConverter::decode_internal(File &file) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not an XYZ image");

    uint16_t width = file.io.read_u16_le();
    uint16_t height = file.io.read_u16_le();

    std::string data = zlib_inflate(file.io.read_until_end());
    assert(data.size() == 256 * 3 + width * height);

    size_t pixels_size = width * height * 3;
    std::unique_ptr<char[]> pixels(new char[pixels_size]);

    const char *palette = data.data();
    const char *palette_indices = data.data() + 256 * 3;
    char *out = pixels.get();

    for (size_t i = 0; i < width * height; i ++)
    {
        size_t index = *palette_indices ++;
        *out ++ = palette[index * 3 + 0];
        *out ++ = palette[index * 3 + 1];
        *out ++ = palette[index * 3 + 2];
    }

    std::unique_ptr<Image> image = Image::from_pixels(
        width,
        height,
        std::string(pixels.get(), pixels_size),
        IMAGE_PIXEL_FORMAT_RGB);
    image->update_file(file);
}
