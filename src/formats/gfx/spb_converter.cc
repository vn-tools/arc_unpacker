// SPB image
//
// Company:   -
// Engine:    Nscripter
// Extension: -
// Archives:  NSA
//
// Known games:
// - Umineko no naku koro ni

#include "bit_reader.h"
#include "formats/gfx/spb_converter.h"
#include "formats/image.h"

namespace
{
    typedef unsigned char uchar;

    std::unique_ptr<uchar> decode_pixels(
        size_t image_width,
        size_t image_height,
        BitReader &bit_reader,
        size_t &output_size)
    {
        output_size = image_width * image_height * 3;
        std::unique_ptr<uchar> output(new uchar[output_size]);

        size_t channel_data_size = image_width * image_height;
        std::unique_ptr<uchar> channel_data(new uchar[channel_data_size]);
        for (ssize_t rgb = 2; rgb >= 0; rgb--)
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

void SpbConverter::decode_internal(VirtualFile &file) const
{
    uint16_t width = file.io.read_u16_be();
    uint16_t height = file.io.read_u16_be();

    if (height == 0 || width == 0)
        throw std::runtime_error("Not a SPB image");

    if (static_cast<uint32_t>(width * height) > 0x0fffffff)
        throw std::runtime_error("Image is too big");

    size_t uncompressed_size = file.io.size() - file.io.tell();
    std::unique_ptr<char> uncompressed(new char[uncompressed_size]);
    file.io.read(uncompressed.get(), uncompressed_size);
    BitReader bit_reader(uncompressed.get(), uncompressed_size);

    size_t uncompressed_data_size;
    std::unique_ptr<uchar> uncompressed_data
        = decode_pixels(width, height, bit_reader, uncompressed_data_size);

    std::unique_ptr<Image> image = Image::from_pixels(
        width,
        height,
        std::string(
            reinterpret_cast<char*>(uncompressed_data.get()),
            uncompressed_data_size),
        IMAGE_PIXEL_FORMAT_RGB);
    image->update_file(file);
}
