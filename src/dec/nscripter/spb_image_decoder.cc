#include "dec/nscripter/spb_image_decoder.h"
#include "algo/range.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::nscripter;

static res::Image decode_image(
    size_t width, size_t height, io::BaseBitStream &bit_stream)
{
    res::Image output(width, height);
    for (auto &c : output)
        c.a = 0xFF;

    bstr channel_data(width * height);

    for (int rgb = 2; rgb >= 0; rgb--)
    {
        u8 *channel_ptr = channel_data.get<u8>();
        const u8 *channel_end = channel_ptr + channel_data.size();

        u8 ch = bit_stream.read(8);
        if (channel_ptr >= channel_end) break;
        *channel_ptr++ = ch;

        while (channel_ptr < channel_end)
        {
            size_t t = bit_stream.read(3);
            if (t == 0)
            {
                if (channel_ptr >= channel_end)
                    break;
                *channel_ptr++ = ch;
                if (channel_ptr >= channel_end)
                    break;
                *channel_ptr++ = ch;
                if (channel_ptr >= channel_end)
                    break;
                *channel_ptr++ = ch;
                if (channel_ptr >= channel_end)
                    break;
                *channel_ptr++ = ch;
                continue;
            }

            size_t mask = t == 7 ? bit_stream.read(1) + 1 : t + 2;

            for (auto i : algo::range(4))
            {
                if (mask == 8)
                {
                    ch = bit_stream.read(8);
                }
                else
                {
                    t = bit_stream.read(mask);
                    if (t & 1)
                        ch += (t >> 1) + 1;
                    else
                        ch -= (t >> 1);
                }
                if (channel_ptr >= channel_end)
                    break;
                *channel_ptr++ = ch;
            }
        }

        const u8 *p = channel_data.get<const u8>();
        for (auto y : algo::range(0, height))
        {
            if (y & 1)
            {
                for (auto x : algo::range(width))
                    output.at(width - 1 - x, y)[2 - rgb] = *p++;
            }
            else
            {
                for (auto x : algo::range(width))
                    output.at(x, y)[2 - rgb] = *p++;
            }
        }
    }

    return output;
}

bool SpbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("bmp"))
        return false;
    const auto width = input_file.stream.read_be<u16>();
    const auto height = input_file.stream.read_be<u16>();
    if (height == 0 || width == 0)
        return false;
    if (width > 5000 || height > 5000)
        return false;
    return true;
}

res::Image SpbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto width = input_file.stream.read_be<u16>();
    const auto height = input_file.stream.read_be<u16>();
    io::MsbBitStream bit_stream(input_file.stream.read_to_eof());
    return decode_image(width, height, bit_stream);
}

static auto _ = dec::register_decoder<SpbImageDecoder>("nscripter/spb");
