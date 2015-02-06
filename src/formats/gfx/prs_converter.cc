// PRS image
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: -
// Archives:  MBL

#include <cassert>
#include <cstdint>
#include <cstring>
#include "formats/gfx/prs_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"

namespace
{
    const char *prs_magic = "YB\x83\x03";
    const size_t prs_magic_length = 4;

    bool prs_decode_pixels(
        uint16_t image_width,
        uint16_t image_height,
        const char *source_buffer,
        const size_t source_size,
        char **target_buffer,
        size_t *target_size)
    {
        size_t i;
        assert(source_buffer != nullptr);
        assert(target_buffer != nullptr);

        *target_size = image_width * image_height * 3;
        *target_buffer = new char[*target_size];
        assert(*target_buffer != nullptr);

        const unsigned char *source = (const unsigned char*)source_buffer;
        const unsigned char *source_guardian = source + source_size;
        unsigned char *target = (unsigned char*)(*target_buffer);
        unsigned char *target_guardian = target + *target_size;

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
                    assert(target - shift >= (unsigned char*)*target_buffer);
                    *target = *(target - shift);
                    target ++;
                }
            }
        }

        for (i = 3; i < *target_size; i ++)
            (*target_buffer)[i] += (*target_buffer)[i-3];
        return true;
    }

    bool prs_check_magic(IO *io)
    {
        assert(io != nullptr);
        char magic[prs_magic_length];
        io_read_string(io, magic, prs_magic_length);
        return memcmp(magic, prs_magic, prs_magic_length) == 0;
    }
}

bool PrsConverter::decode_internal(VirtualFile *file)
{
    assert(file != nullptr);

    if (!prs_check_magic(file->io))
    {
        log_error("Not a PRS graphic file");
        return false;
    }

    uint32_t source_size = io_read_u32_le(file->io);
    io_skip(file->io, 4);
    uint16_t image_width = io_read_u16_le(file->io);
    uint16_t image_height = io_read_u16_le(file->io);

    char *source_buffer = new char[source_size];
    io_read_string(file->io, source_buffer, source_size);

    bool result;
    char *target_buffer = nullptr;
    size_t target_size = 0;
    if (!prs_decode_pixels(
        image_width,
        image_height,
        source_buffer,
        source_size,
        &target_buffer,
        &target_size))
    {
        result = false;
    }
    else
    {
        Image *image = image_create_from_pixels(
            image_width,
            image_height,
            target_buffer,
            target_size,
            IMAGE_PIXEL_FORMAT_BGR);
        image_update_file(image, file);
        image_destroy(image);
        result = true;
    }

    delete []source_buffer;
    delete []target_buffer;
    return result;
}
