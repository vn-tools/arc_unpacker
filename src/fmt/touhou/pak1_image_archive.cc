// PAK1 image file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 07.5 - Immaterial and Missing Power

#include "fmt/touhou/pak1_image_archive.h"
#include "fmt/touhou/palette.h"
#include "io/buffered_io.h"
#include "util/colors.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static std::unique_ptr<File> read_image(
    io::IO &arc_io, size_t index, Palette palette)
{
    auto width = arc_io.read_u32_le();
    auto height = arc_io.read_u32_le();
    arc_io.skip(4);
    auto bit_depth = arc_io.read_u8();
    size_t source_size = arc_io.read_u32_le();

    io::BufferedIO source_io(arc_io, source_size);

    std::vector<util::Color> pixels(width * height);
    auto *pixels_ptr = &pixels[0];
    while (source_io.tell() < source_io.size())
    {
        size_t repeat;
        util::Color color;

        switch (bit_depth)
        {
            case 32:
                repeat = source_io.read_u32_le();
                color = util::color::bgra8888(source_io);
                break;

            case 24:
                repeat = source_io.read_u32_le();
                color = util::color::bgr888(source_io);
                source_io.skip(1);
                break;

            case 16:
                repeat = source_io.read_u16_le();
                color = util::color::bgra5551(source_io);
                break;

            case 8:
                repeat = source_io.read_u8();
                color = palette[source_io.read_u8()];
                break;

            default:
                throw std::runtime_error("Unsupported channel count");
        }

        while (repeat--)
            *pixels_ptr++ = color;
    }

    auto image = util::Image::from_pixels(width, height, pixels);
    return image->create_file(util::format("%04d", index));
}

bool Pak1ImageArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("dat");
}

void Pak1ImageArchive::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    auto palette_count = arc_file.io.read_u8();
    std::vector<Palette> palettes;
    for (auto p : util::range(palette_count))
    {
        Palette palette;
        for (auto i : util::range(256))
            palette[i] = util::color::bgra5551(arc_file.io);
        palettes.push_back(palette);
    }
    palettes.push_back(create_default_palette());

    size_t i = 0;
    while (arc_file.io.tell() < arc_file.io.size())
        file_saver.save(read_image(arc_file.io, i++, palettes[0]));
}
