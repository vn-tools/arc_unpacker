// PAK1 image file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 07.5 - Immaterial and Missing Power

#include "buffered_io.h"
#include "formats/image.h"
#include "formats/touhou/pak1_image_archive.h"
#include "util/colors.h"
#include "util/itos.h"
using namespace Formats::Touhou;

namespace
{
    std::unique_ptr<File> read_image(
        IO &arc_io, size_t index, uint32_t *palette)
    {
        auto image_width = arc_io.read_u32_le();
        auto image_height = arc_io.read_u32_le();
        arc_io.skip(4);
        auto bit_depth = arc_io.read_u8();
        size_t source_size = arc_io.read_u32_le();
        size_t target_size = image_width * image_height * 4;

        BufferedIO target_io;
        target_io.reserve(target_size);
        BufferedIO source_io;
        source_io.write_from_io(arc_io, source_size);
        source_io.seek(0);

        while (source_io.tell() < source_io.size())
        {
            size_t repeat;
            uint32_t rgba;

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
                        | 0xff000000;
                    source_io.skip(1);
                    break;

                case 16:
                    repeat = source_io.read_u16_le();
                    rgba = rgba5551(source_io.read_u16_le());
                    break;

                case 8:
                    repeat = source_io.read_u8();
                    rgba = palette[static_cast<size_t>(source_io.read_u8())];
                    break;

                default:
                    throw std::runtime_error("Unsupported channel count");
            }

            while (repeat --)
                target_io.write_u32_le(rgba);
        }

        std::unique_ptr<File> file(new File);
        file->name = itos(index, 4);
        target_io.seek(0);
        auto image = Image::from_pixels(
            image_width,
            image_height,
            target_io.read(target_io.size()),
            IMAGE_PIXEL_FORMAT_BGRA);
        image->update_file(*file);
        return file;
    }
}

void Pak1ImageArchive::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    auto palette_size = arc_file.io.read_u8();
    std::unique_ptr<uint32_t[]> palette(new uint32_t[256 * palette_size]);
    for (size_t p = 0; p < palette_size; p ++)
        for (size_t i = 0; i < 256; i ++)
            palette[p * 256 + i] = rgba5551(arc_file.io.read_u16_le());

    size_t i = 0;
    while (arc_file.io.tell() < arc_file.io.size())
        file_saver.save(read_image(arc_file.io, i ++, palette.get()));
}
