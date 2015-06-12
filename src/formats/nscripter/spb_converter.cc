// SPB image
//
// Company:   -
// Engine:    NScripter
// Extension: -
// Archives:  NSA
//
// Known games:
// - Umineko no naku koro ni

#include "formats/nscripter/spb_converter.h"
#include "io/bit_reader.h"
#include "util/image.h"
using namespace Formats::NScripter;

namespace
{
    typedef unsigned char uchar;

    std::unique_ptr<uchar[]> decode_pixels(
        size_t image_width,
        size_t image_height,
        BitReader &bit_reader,
        size_t &output_size)
    {
        output_size = image_width * image_height * 3;
        std::unique_ptr<uchar[]> output(new uchar[output_size]);

        size_t channel_data_size = image_width * image_height;
        std::unique_ptr<uchar[]> channel_data(new uchar[channel_data_size]);
        for (int rgb = 2; rgb >= 0; rgb--)
        {
            uchar *channel_ptr = channel_data.get();
            const uchar *channel_guardian = channel_ptr + channel_data_size;

            uchar ch = bit_reader.try_get(8);
            if (channel_ptr >= channel_guardian) break;
            *channel_ptr ++ = ch;

            while (channel_ptr < channel_guardian)
            {
                size_t t = bit_reader.try_get(3);
                if (t == 0)
                {
                    if (channel_ptr >= channel_guardian) break;
                    *channel_ptr ++ = ch;
                    if (channel_ptr >= channel_guardian) break;
                    *channel_ptr ++ = ch;
                    if (channel_ptr >= channel_guardian) break;
                    *channel_ptr ++ = ch;
                    if (channel_ptr >= channel_guardian) break;
                    *channel_ptr ++ = ch;
                    continue;
                }

                size_t mask = t == 7 ? bit_reader.try_get(1) + 1 : t + 2;

                for (size_t i = 0; i < 4; i++)
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
                    *channel_ptr ++ = ch;
                }
            }

            const uchar *p = channel_data.get();
            uchar *q = output.get() + rgb;
            for (size_t j = 0; j < image_height >> 1; j++)
            {
                for (size_t i = 0; i < image_width; i++)
                {
                    *q = *p++;
                    q += 3;
                }
                q += image_width * 3;
                for (size_t i = 0; i < image_width; i++)
                {
                    q -= 3;
                    *q = *p++;
                }
                q += image_width * 3;
            }
            if (image_height & 1)
            {
                for (size_t i = 0; i < image_width; i++)
                {
                    *q = *p++;
                    q += 3;
                }
            }
        }

        return output;
    }
}

bool SpbConverter::is_recognized_internal(File &file) const
{
    if (!file.has_extension("bmp"))
        return false;
    uint16_t width = file.io.read_u16_be();
    uint16_t height = file.io.read_u16_be();
    if (height == 0 || width == 0)
        return false;
    if (static_cast<uint32_t>(width * height) > 0x0fffffff)
        return false;
    return true;
}

std::unique_ptr<File> SpbConverter::decode_internal(File &file) const
{
    uint16_t width = file.io.read_u16_be();
    uint16_t height = file.io.read_u16_be();

    size_t uncompressed_size = file.io.size() - file.io.tell();
    std::unique_ptr<char[]> uncompressed(new char[uncompressed_size]);
    file.io.read(uncompressed.get(), uncompressed_size);
    BitReader bit_reader(uncompressed.get(), uncompressed_size);

    size_t uncompressed_data_size;
    std::unique_ptr<uchar[]> uncompressed_data
        = decode_pixels(width, height, bit_reader, uncompressed_data_size);

    std::unique_ptr<Image> image = Image::from_pixels(
        width,
        height,
        std::string(
            reinterpret_cast<char*>(uncompressed_data.get()),
            uncompressed_data_size),
        PixelFormat::RGB);
    return image->create_file(file.name);
}
