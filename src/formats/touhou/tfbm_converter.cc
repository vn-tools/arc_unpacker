// TFBM image file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .png, .bmp
//
// Known games:
// - Touhou 13.5 - Hopeless Masquerade

#include "buffered_io.h"
#include "formats/image.h"
#include "formats/touhou/tfbm_converter.h"
#include "util/colors.h"
#include "util/itos.h"
#include "util/zlib.h"
using namespace Formats::Touhou;

namespace
{
    const std::string magic("TFBM\x00", 5);
}

struct TfbmConverter::Internals
{
    PaletteMap palette_map;
};

TfbmConverter::TfbmConverter() : internals(new Internals)
{
}

TfbmConverter::~TfbmConverter()
{
}

void TfbmConverter::set_palette_map(const PaletteMap &palette_map)
{
    internals->palette_map.clear();
    for (auto &it : palette_map)
        internals->palette_map[it.first] = it.second;
}

void TfbmConverter::decode_internal(File &file) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a TFBM image file");

    auto bit_depth = file.io.read_u8();
    auto image_width = file.io.read_u32_le();
    auto image_height = file.io.read_u32_le();
    auto image_width_padded = file.io.read_u32_le();
    auto source_size = file.io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    BufferedIO target_io;
    target_io.reserve(target_size);
    BufferedIO source_io(zlib_inflate(file.io.read_until_end()));

    Palette palette;
    if (bit_depth == 8)
    {
        uint32_t palette_number = 0;
        auto path = boost::filesystem::path(file.name);
        path.remove_filename();
        path /= "palette" + itos(palette_number, 3) + ".bmp";

        auto it = internals->palette_map.find(path.generic_string());
        palette = it != internals->palette_map.end()
            ? it->second
            : create_default_palette();
    }

    for (size_t y = 0; y < image_height; y ++)
    {
        for (size_t x = 0; x < image_width_padded; x ++)
        {
            uint32_t rgba;

            switch (bit_depth)
            {
                case 32:
                case 24:
                    rgba = source_io.read_u32_le();
                    break;

                case 8:
                    rgba = palette[source_io.read_u8()];
                    break;

                default:
                    throw std::runtime_error("Unsupported channel count");
            }

            if (x < image_width)
                target_io.write_u32_le(rgba);
        }
    }

    target_io.seek(0);
    auto image = Image::from_pixels(
        image_width,
        image_height,
        target_io.read(target_io.size()),
        PixelFormat::BGRA);
    image->update_file(file);
}

