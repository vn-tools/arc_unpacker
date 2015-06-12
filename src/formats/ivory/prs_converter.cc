// PRS image
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: -
// Archives:  MBL

#include "formats/ivory/prs_converter.h"
#include "util/image.h"
using namespace Formats::Ivory;

namespace
{
    const std::string magic("YB", 2);

    void decode_pixels(
        const u8 *source,
        const size_t source_size,
        u8 *target,
        const size_t target_size)
    {
        const u8 *source_ptr = source;
        const u8 *source_guardian = source + source_size;
        u8 *target_ptr = target;
        u8 *target_guardian = target + target_size;

        int flag = 0;
        int length_lookup[256];
        for (size_t i = 0; i < 256; i++)
            length_lookup[i] = i + 3;
        length_lookup[0xff] = 0x1000;
        length_lookup[0xfe] = 0x400;
        length_lookup[0xfd] = 0x100;

        while (1)
        {
            flag <<= 1;
            if ((flag & 0xff) == 0)
            {
                if (source_ptr >= source_guardian)
                    break;
                flag = *source_ptr++;
                flag <<= 1;
                flag += 1;
            }

            if ((flag & 0x100) != 0x100)
            {
                if (source_ptr >= source_guardian
                    || target_ptr >= target_guardian)
                {
                    break;
                }

                *target_ptr++ = *source_ptr++;
            }
            else
            {
                int tmp = *source_ptr++;
                size_t length = 0;
                size_t shift = 0;

                if (tmp & 0x80)
                {
                    if (source_ptr >= source_guardian)
                        break;

                    shift = (*source_ptr++) | ((tmp & 0x3f) << 8);
                    if (tmp & 0x40)
                    {
                        if (source_ptr >= source_guardian)
                            break;
                        auto index = static_cast<size_t>(*source_ptr++);
                        length = length_lookup[index];
                    }
                    else
                    {
                        length = (shift & 0xf) + 3;
                        shift >>= 4;
                    }
                }
                else
                {
                    length = tmp >> 2;
                    tmp &= 3;
                    if (tmp == 3)
                    {
                        length += 9;
                        for (size_t i = 0; i < length; i++)
                        {
                            if (source_ptr >= source_guardian
                                || target_ptr >= target_guardian)
                            {
                                break;
                            }
                            *target_ptr++ = *source_ptr++;
                        }
                        continue;
                    }
                    shift = length;
                    length = tmp + 2;
                }

                shift += 1;
                for (size_t i = 0; i < length; i++)
                {
                    if (target_ptr >= target_guardian)
                        break;
                    if (target_ptr - shift < target)
                        throw std::runtime_error("Invalid shift value");
                    *target_ptr = *(target_ptr - shift);
                    target_ptr++;
                }
            }
        }
    }
}

bool PrsConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PrsConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    bool using_differences = file.io.read_u8() > 0;
    if (file.io.read_u8() != 3)
        throw std::runtime_error("Unknown PRS version");

    u32 source_size = file.io.read_u32_le();
    file.io.skip(4);
    u16 image_width = file.io.read_u16_le();
    u16 image_height = file.io.read_u16_le();

    const size_t target_size = image_width * image_height * 3;
    std::unique_ptr<char[]> source(new char[source_size]);
    std::unique_ptr<char[]> target(new char[target_size]);
    file.io.read(source.get(), source_size);

    decode_pixels(
        reinterpret_cast<u8*>(source.get()), source_size,
        reinterpret_cast<u8*>(target.get()), target_size);

    if (using_differences)
    {
        for (size_t i = 3; i < target_size; i++)
            target[i] += target[i - 3];
    }

    std::unique_ptr<Image> image = Image::from_pixels(
        image_width,
        image_height,
        std::string(target.get(), target_size),
        PixelFormat::BGR);
    return image->create_file(file.name);
}
