// PGX image
//
// Company:   Rune
// Engine:    GLib
// Extension: -
// Archives:  GLib2
//
// Known games:
// - Musume Shimai
// - Watashi no Puni Puni
// - Pure My Imouto Milk Purin

#include "fmt/glib/gml_decoder.h"
#include "fmt/glib/pgx_converter.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::glib;

static const std::string magic = "PGX\x00"_s;

bool PgxConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PgxConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    file.io.skip(4);
    size_t image_width = file.io.read_u32_le();
    size_t image_height = file.io.read_u32_le();
    bool transparent = file.io.read_u16_le();
    file.io.skip(2);
    size_t source_size = file.io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    io::BufferedIO source_io;
    io::BufferedIO target_io;
    file.io.seek(file.io.size() - source_size);
    source_io.write_from_io(file.io, source_size);
    source_io.seek(0);
    target_io.reserve(target_size);

    GmlDecoder::decode(source_io, target_io);

    if (!transparent)
    {
        u8 *buffer = reinterpret_cast<u8*>(target_io.buffer());
        u8 *buffer_guardian = buffer + target_io.size();
        buffer += 3;
        while (buffer < buffer_guardian)
        {
            *buffer = 0xFF;
            buffer += 4;
        }
    }

    target_io.seek(0);

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        image_width,
        image_height,
        target_io.read_until_end(),
        util::PixelFormat::BGRA);
    return image->create_file(file.name);
}
