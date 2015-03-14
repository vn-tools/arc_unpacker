// PGX image
//
// Company:   Rune
// Engine:    GLib
// Extension: -
// Archives:  GLib2

#include "buffered_io.h"
#include "formats/image.h"
#include "formats/glib/pgx_converter.h"
using namespace Formats::Glib;

namespace
{
    const std::string magic("PGX\x00", 4);

    // modified LZSS
    void decode_pixels(
        BufferedIO &source_io, BufferedIO &target_io, bool transparent)
    {
        std::unique_ptr<char[]> target(new char[target_io.size()]);
        char *target_ptr = target.get();
        char *target_guardian = target.get() + target_io.size();

        int unk1 = 0;
        while (target_ptr < target_guardian)
        {
            int carry = unk1 & 1;
            unk1 >>= 1;
            if (!unk1)
            {
                unk1 = source_io.read_u8();
                carry = unk1 & 1;
                unk1 = (unk1 >> 1) | 0x80;
            }

            if (carry)
            {
                *target_ptr ++ = source_io.read_u8();
                continue;
            }

            uint8_t tmp1 = source_io.read_u8();
            uint8_t tmp2 = source_io.read_u8();
            size_t length = ((~tmp2) & 0xf) + 3;
            int32_t look_behind = (tmp1 | ((tmp2 << 4) & 0xf00)) + 18;
            look_behind -= target_ptr - target.get();
            look_behind |= 0xfffff000;

            while (target_ptr - target.get() + look_behind < 0 && length)
            {
                -- length;
                *target_ptr ++ = 0;
            }

            while (length)
            {
                -- length;
                uint8_t tmp = target_ptr[look_behind];
                *target_ptr ++ = tmp;
            }
        }

        if (!transparent)
        {
            for (size_t i = 3; i < target_io.size(); i += 4)
                target[i] = 0xff;
        }
        target_io.write(target.get(), target_io.size());
    }
}

void PgxConverter::decode_internal(File &file) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a PGX image");

    file.io.skip(4);
    size_t image_width = file.io.read_u32_le();
    size_t image_height = file.io.read_u32_le();
    bool transparent = file.io.read_u16_le();
    file.io.skip(2);
    size_t source_size = file.io.read_u32_le();
    size_t target_size = image_width * image_height * 4;

    BufferedIO source_io;
    BufferedIO target_io;
    file.io.seek(file.io.size() - source_size);
    source_io.write_from_io(file.io, source_size);
    source_io.seek(0);
    target_io.reserve(target_size);

    decode_pixels(source_io, target_io, transparent);
    target_io.seek(0);

    std::unique_ptr<Image> image = Image::from_pixels(
        image_width,
        image_height,
        target_io.read_until_end(),
        IMAGE_PIXEL_FORMAT_BGRA);
    image->update_file(file);
}
