// PAK2 image file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .cv2
//
// Known games:
// - Touhou 10.5 - Scarlet Weather Rhapsody
// - Touhou 12.3 - Unthinkable Natural Law

#include <boost/filesystem.hpp>
#include "buffered_io.h"
#include "formats/image.h"
#include "formats/touhou/pak2_image_converter.h"
#include "util/colors.h"
#include "util/itos.h"
using namespace Formats::Touhou;

struct Pak2ImageConverter::Internals
{
    PaletteMap palette_map;

    Internals(PaletteMap &palette_map) : palette_map(std::move(palette_map))
    {
    }

    ~Internals()
    {
    }
};

Pak2ImageConverter::Pak2ImageConverter(PaletteMap &palette_map)
    : internals(new Internals(palette_map))
{
}

Pak2ImageConverter::~Pak2ImageConverter()
{
}

void Pak2ImageConverter::decode_internal(File &file) const
{
    if (file.name.find("cv2") == std::string::npos)
        throw std::runtime_error("Not a CV2 image file");

    auto bit_depth = file.io.read_u8();
    auto image_width = file.io.read_u32_le();
    auto image_height = file.io.read_u32_le();
    auto image_width_padded = file.io.read_u32_le();
    auto palette_number = file.io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    BufferedIO target_io;
    target_io.reserve(target_size);
    BufferedIO source_io;
    source_io.write_from_io(file.io);
    source_io.seek(0);

    uint32_t *palette = nullptr;
    if (bit_depth == 8)
    {
        auto path = boost::filesystem::path(file.name);
        path.remove_filename();
        path /= "palette" + itos(palette_number, 3) + ".pal";
        palette = internals->palette_map[path.generic_string()].get();
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
                {
                    auto index = source_io.read_u8();
                    rgba = palette == nullptr
                        ? rgba_gray(index)
                        : palette[index];
                    break;
                }

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
