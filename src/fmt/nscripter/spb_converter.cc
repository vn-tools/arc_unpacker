// SPB image
//
// Company:   -
// Engine:    NScripter
// Extension: -
// Archives:  NSA
//
// Known games:
// - [07th Expansion & Alchemist] [070817] Umineko no Naku Koro ni 1-4

#include "fmt/nscripter/spb_converter.h"
#include "io/bit_reader.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nscripter;

static std::unique_ptr<pix::Grid> decode_pixels(
    size_t width, size_t height, io::BitReader &bit_reader)
{
    std::unique_ptr<pix::Grid> output(new pix::Grid(width, height));
    for (auto y : util::range(height))
    for (auto x : util::range(width))
        output->at(x, y).a = 0xFF;

    bstr channel_data;
    channel_data.resize(width * height);

    for (int rgb = 2; rgb >= 0; rgb--)
    {
        u8 *channel_ptr = channel_data.get<u8>();
        const u8 *channel_end = channel_ptr + channel_data.size();

        u8 ch = bit_reader.get(8);
        if (channel_ptr >= channel_end) break;
        *channel_ptr++ = ch;

        while (channel_ptr < channel_end)
        {
            size_t t = bit_reader.get(3);
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

            size_t mask = t == 7 ? bit_reader.get(1) + 1 : t + 2;

            for (auto i : util::range(4))
            {
                if (mask == 8)
                {
                    ch = bit_reader.get(8);
                }
                else
                {
                    t = bit_reader.get(mask);
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
        for (auto y : util::range(0, height))
        {
            if (y & 1)
            {
                for (auto x : util::range(width))
                    output->at(width - 1 - x, y)[2 - rgb] = *p++;
            }
            else
            {
                for (auto x : util::range(width))
                    output->at(x, y)[2 - rgb] = *p++;
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

    io::BitReader bit_reader(file.io.read_to_eof());
    auto pixels = decode_pixels(width, height, bit_reader);
    return util::Image::from_pixels(*pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<SpbConverter>("nscripter/spb");
