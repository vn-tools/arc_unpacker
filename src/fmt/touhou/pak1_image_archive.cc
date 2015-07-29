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

using namespace au;
using namespace au::fmt::touhou;

static std::unique_ptr<File> read_image(
    io::IO &arc_io, size_t index, Palette palette)
{
    auto image_width = arc_io.read_u32_le();
    auto image_height = arc_io.read_u32_le();
    arc_io.skip(4);
    auto bit_depth = arc_io.read_u8();
    size_t source_size = arc_io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    io::BufferedIO target_io;
    target_io.reserve(target_size);
    io::BufferedIO source_io;
    source_io.write_from_io(arc_io, source_size);
    source_io.seek(0);

    while (source_io.tell() < source_io.size())
    {
        size_t repeat;
        u32 rgba;

        switch (bit_depth)
        {
            case 32:
                repeat = source_io.read_u32_le();
                rgba = source_io.read_u32_le();
                break;

            case 24:
                repeat = source_io.read_u32_le();
                rgba = source_io.read_u8()
                    | (source_io.read_u8() << 8)
                    | (source_io.read_u8() << 16)
                    | 0xFF000000;
                source_io.skip(1);
                break;

            case 16:
                repeat = source_io.read_u16_le();
                rgba = util::color::rgba5551(source_io.read_u16_le());
                break;

            case 8:
                repeat = source_io.read_u8();
                rgba = palette[static_cast<size_t>(source_io.read_u8())];
                break;

            default:
                throw std::runtime_error("Unsupported channel count");
        }

        while (repeat--)
            target_io.write_u32_le(rgba);
    }

    target_io.seek(0);
    auto image = util::Image::from_pixels(
        image_width,
        image_height,
        target_io.read(target_io.size()),
        util::PixelFormat::BGRA);
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
    for (size_t p = 0; p < palette_count; p++)
    {
        Palette palette;
        for (size_t i = 0; i < 256; i++)
            palette[i] = util::color::rgba5551(arc_file.io.read_u16_le());
        palettes.push_back(palette);
    }
    palettes.push_back(create_default_palette());

    size_t i = 0;
    while (arc_file.io.tell() < arc_file.io.size())
        file_saver.save(read_image(arc_file.io, i++, palettes[0]));
}
