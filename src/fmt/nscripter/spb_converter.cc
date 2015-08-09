// SPB image
//
// Company:   -
// Engine:    NScripter
// Extension: -
// Archives:  NSA
//
// Known games:
// - Umineko no naku koro ni

#include "fmt/nscripter/spb_converter.h"
#include "io/bit_reader.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nscripter;

static bstr decode_pixels(
    size_t width, size_t height, io::BitReader &bit_reader)
{
    bstr output;
    output.resize(width * height * 3);

    bstr channel_data;
    channel_data.resize(width * height);

    for (int rgb = 2; rgb >= 0; rgb--)
    {
        u8 *channel_ptr = channel_data.get<u8>();
        const u8 *channel_guardian = channel_ptr + channel_data.size();

        u8 ch = bit_reader.try_get(8);
        if (channel_ptr >= channel_guardian) break;
        *channel_ptr++ = ch;

        while (channel_ptr < channel_guardian)
        {
            size_t t = bit_reader.try_get(3);
            if (t == 0)
            {
                if (channel_ptr >= channel_guardian) break;
                *channel_ptr++ = ch;
                if (channel_ptr >= channel_guardian) break;
                *channel_ptr++ = ch;
                if (channel_ptr >= channel_guardian) break;
                *channel_ptr++ = ch;
                if (channel_ptr >= channel_guardian) break;
                *channel_ptr++ = ch;
                continue;
            }

            size_t mask = t == 7 ? bit_reader.try_get(1) + 1 : t + 2;

            for (auto i : util::range(4))
            {
                if (mask == 8)
                {
                    ch = bit_reader.try_get(8);
                }
                else
                {
                    t = bit_reader.try_get(mask);
                    if (t & 1)
                        ch += (t >> 1) + 1;
                    else
                        ch -= (t >> 1);
                }
                if (channel_ptr >= channel_guardian) break;
                *channel_ptr++ = ch;
            }
        }

        const u8 *p = channel_data.get<const u8>();
        u8 *q = &output.get<u8>()[rgb];
        for (auto j : util::range(height >> 1))
        {
            for (auto i : util::range(width))
            {
                *q = *p++;
                q += 3;
            }
            q += width * 3;
            for (auto i : util::range(width))
            {
                q -= 3;
                *q = *p++;
            }
            q += width * 3;
        }
        if (height & 1)
        {
            for (auto i : util::range(width))
            {
                *q = *p++;
                q += 3;
            }
        }
    }

    return output;
}

bool SpbConverter::is_recognized_internal(File &file) const
{
    if (!file.has_extension("bmp"))
        return false;
    u16 width = file.io.read_u16_be();
    u16 height = file.io.read_u16_be();
    if (height == 0 || width == 0)
        return false;
    if (static_cast<u32>(width * height) > 0x0FFFFFFF)
        return false;
    return true;
}

std::unique_ptr<File> SpbConverter::decode_internal(File &file) const
{
    u16 width = file.io.read_u16_be();
    u16 height = file.io.read_u16_be();

    io::BitReader bit_reader(file.io.read_until_end());
    auto uncompressed_data = decode_pixels(width, height, bit_reader);
    auto image = util::Image::from_pixels(
        width, height, uncompressed_data, util::PixelFormat::RGB);
    return image->create_file(file.name);
}
