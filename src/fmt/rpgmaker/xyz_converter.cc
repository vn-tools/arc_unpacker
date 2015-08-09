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
#include "fmt/rpgmaker/xyz_converter.h"
#include "util/image.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "XYZ1"_b;

bool XyzConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> XyzConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();

    bstr data = util::pack::zlib_inflate(file.io.read_until_end());
    if (data.size() != static_cast<size_t>(256 * 3 + width * height))
        throw std::runtime_error("Unexpected data size");

    bstr pixels;
    pixels.resize(width * height * 3);

    auto *palette = data.get<const u8>();
    auto *palette_indices = data.get<const u8>() + 256 * 3;
    auto *pixels_ptr = pixels.get<u8>();

    for (auto i : util::range(width * height))
    {
        size_t index = *palette_indices++;
        for (auto c : util::range(3))
            *pixels_ptr++ = palette[index * 3 + c];
    }

    auto image = util::Image::from_pixels(
        width, height, pixels, util::PixelFormat::RGB);
    return image->create_file(file.name);
}
