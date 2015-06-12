// XYZ image
//
// Company:   Enterbrain
// Engine:    RPGMaker
// Extension: .xyz
// Archives:  -
//
// Known games:
// - Yume Nikki

#include <stdexcept>
#include "formats/rpgmaker/xyz_converter.h"
#include "util/image.h"
#include "util/zlib.h"
using namespace Formats::RpgMaker;

namespace
{
    const std::string magic("XYZ1", 4);
}

bool XyzConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> XyzConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();

    std::string data = zlib_inflate(file.io.read_until_end());
    if (data.size() != static_cast<size_t>(256 * 3 + width * height))
        throw std::runtime_error("Unexpected data size");

    size_t pixels_size = width * height * 3;
    std::unique_ptr<char[]> pixels(new char[pixels_size]);

    const char *palette = data.data();
    const char *palette_indices = data.data() + 256 * 3;
    char *out = pixels.get();

    for (size_t i = 0; i < static_cast<size_t>(width * height); i++)
    {
        size_t index = *palette_indices++;
        *out++ = palette[index * 3 + 0];
        *out++ = palette[index * 3 + 1];
        *out++ = palette[index * 3 + 2];
    }

    std::unique_ptr<Image> image = Image::from_pixels(
        width,
        height,
        std::string(pixels.get(), pixels_size),
        PixelFormat::RGB);
    return image->create_file(file.name);
}
