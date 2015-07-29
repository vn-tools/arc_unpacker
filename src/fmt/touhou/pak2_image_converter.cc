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
#include "fmt/touhou/pak2_image_converter.h"
#include "io/buffered_io.h"
#include "util/colors.h"
#include "util/format.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::touhou;

struct Pak2ImageConverter::Priv
{
    PaletteMap palette_map;
};

Pak2ImageConverter::Pak2ImageConverter() : p(new Priv)
{
}

Pak2ImageConverter::~Pak2ImageConverter()
{
}

void Pak2ImageConverter::set_palette_map(const PaletteMap &palette_map)
{
    p->palette_map.clear();
    for (auto &it : palette_map)
        p->palette_map[it.first] = it.second;
}

bool Pak2ImageConverter::is_recognized_internal(File &file) const
{
    return file.has_extension("cv2");
}

std::unique_ptr<File> Pak2ImageConverter::decode_internal(File &file) const
{
    auto bit_depth = file.io.read_u8();
    auto image_width = file.io.read_u32_le();
    auto image_height = file.io.read_u32_le();
    auto image_width_padded = file.io.read_u32_le();
    auto palette_number = file.io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    io::BufferedIO target_io;
    target_io.reserve(target_size);
    io::BufferedIO source_io;
    source_io.write_from_io(file.io);
    source_io.seek(0);

    Palette palette;
    if (bit_depth == 8)
    {
        auto path = boost::filesystem::path(file.name);
        path.remove_filename();
        path /= util::format("palette%03d.pal", palette_number);

        auto it = p->palette_map.find(path.generic_string());
        palette = it != p->palette_map.end()
            ? it->second
            : create_default_palette();
    }

    for (size_t y = 0; y < image_height; y++)
    {
        for (size_t x = 0; x < image_width_padded; x++)
        {
            u32 rgba;

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
    auto image = util::Image::from_pixels(
        image_width,
        image_height,
        target_io.read(target_io.size()),
        util::PixelFormat::BGRA);
    return image->create_file(file.name);
}
