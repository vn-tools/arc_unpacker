// TFBM image file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .png, .bmp
//
// Known games:
// - Touhou 13.5 - Hopeless Masquerade

#include "fmt/touhou/tfbm_converter.h"
#include "io/buffered_io.h"
#include "util/colors.h"
#include "util/format.h"
#include "util/image.h"
#include "util/zlib.h"

using namespace au;
using namespace au::fmt::touhou;

static const std::string magic = "TFBM\x00"_s;

struct TfbmConverter::Priv
{
    PaletteMap palette_map;
};

TfbmConverter::TfbmConverter() : p(new Priv)
{
}

TfbmConverter::~TfbmConverter()
{
}

void TfbmConverter::set_palette_map(const PaletteMap &palette_map)
{
    p->palette_map.clear();
    for (auto &it : palette_map)
        p->palette_map[it.first] = it.second;
}

bool TfbmConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> TfbmConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto bit_depth = file.io.read_u8();
    auto image_width = file.io.read_u32_le();
    auto image_height = file.io.read_u32_le();
    auto image_width_padded = file.io.read_u32_le();
    auto source_size = file.io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    io::BufferedIO target_io;
    target_io.reserve(target_size);
    io::BufferedIO source_io(util::zlib_inflate(file.io.read_until_end()));

    Palette palette;
    if (bit_depth == 8)
    {
        u32 palette_number = 0;
        auto path = boost::filesystem::path(file.name);
        path.remove_filename();
        path /= util::format("palette%03d.bmp", palette_number);

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

                case 16:
                    rgba = util::color::rgb565(source_io.read_u16_le());
                    break;

                case 8:
                    rgba = palette[source_io.read_u8()];
                    break;

                default:
                    throw std::runtime_error(util::format(
                        "Unsupported channel count: %d", bit_depth));
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
