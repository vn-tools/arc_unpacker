// PRS image
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: -
// Archives:  MBL

#include <cassert>
#include "formats/gfx/prs_converter.h"
#include "formats/image.h"
#include "io.h"

namespace
{
    const std::string prs_magic("YB\x83\x03", 4);

    void prs_decode_pixels(
        const char *source_buffer,
        const size_t source_size,
        char *target_buffer,
        const size_t target_size)
    {
        size_t i;
        assert(source_buffer != nullptr);
        assert(target_buffer != nullptr);

        const unsigned char *source = (const unsigned char*)source_buffer;
        const unsigned char *source_guardian = source + source_size;
        unsigned char *target = (unsigned char*)(target_buffer);
        unsigned char *target_guardian = target + target_size;

        int flag = 0;
        int length_lookup[256];
        for (i = 0; i < 256; i ++)
            length_lookup[i] = i + 3;
        length_lookup[0xff] = 0x1000;
        length_lookup[0xfe] = 0x400;
        length_lookup[0xfd] = 0x100;

        while (1)
        {
            flag <<= 1;
            if ((flag & 0xff) == 0)
            {
                if (source >= source_guardian)
                    break;
                flag = *source ++;
                flag <<= 1;
                flag += 1;
            }

            if ((flag & 0x100) != 0x100)
            {
                if (source >= source_guardian || target >= target_guardian)
                    break;

                *target ++ = *source ++;
            }
            else
            {
                int tmp = *source ++;
                size_t length = 0;
                size_t shift = 0;

                if (tmp < 0x80)
                {
                    length = tmp >> 2;
                    tmp &= 3;
                    if (tmp == 3)
                    {
                        length += 9;
                        for (i = 0; i < length; i ++)
                        {
                            if (source >= source_guardian
                            || target >= target_guardian)
                            {
                                break;
                            }
                            *target ++ = *source ++;
                        }
                        continue;
                    }
                    shift = length;
                    length = tmp + 2;
                }
                else
                {
                    if (source >= source_guardian)
                        break;

                    shift = (*source ++) | ((tmp & 0x3f) << 8);
                    if ((tmp & 0x40) == 0)
                    {
                        length = shift;
                        shift >>= 4;
                        length &= 0xf;
                        length += 3;
                    }
                    else
                    {
                        if (source >= source_guardian)
                            break;

                        length = length_lookup[(size_t)*source ++];
                    }
                }

                shift += 1;
                for (i = 0; i < length; i ++)
                {
                    if (target >= target_guardian)
                        break;
                    assert(target - shift >= (unsigned char*)target_buffer);
                    *target = *(target - shift);
                    target ++;
                }
            }
        }

        for (i = 3; i < target_size; i ++)
            target_buffer[i] += target_buffer[i-3];
    }
}

void PrsConverter::decode_internal(VirtualFile &file) const
{
    if (file.io.read(prs_magic.size()) != prs_magic)
        throw std::runtime_error("Not a PRS graphic file");

    uint32_t source_size = file.io.read_u32_le();
    file.io.skip(4);
    uint16_t image_width = file.io.read_u16_le();
    uint16_t image_height = file.io.read_u16_le();

    const size_t target_size = image_width * image_height * 3;
    std::unique_ptr<char> source_buffer(new char[source_size]);
    std::unique_ptr<char> target_buffer(new char[target_size]);
    file.io.read(source_buffer.get(), source_size);

    prs_decode_pixels(
        source_buffer.get(),
        source_size,
        target_buffer.get(),
        target_size);

    std::unique_ptr<Image> image = Image::from_pixels(
        image_width,
        image_height,
        std::string(target_buffer.get(), target_size),
        IMAGE_PIXEL_FORMAT_BGR);
    image->update_file(file);
}
