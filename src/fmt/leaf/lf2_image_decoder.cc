#include "fmt/leaf/lf2_image_decoder.h"
#include "util/pack/lzss.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAF256\x00"_b;

// Modified LZSS routine (the bit shifts proceed in opposite direction)
static bstr custom_lzss_decompress(const bstr &input, size_t output_size)
{
    bstr output(output_size);

    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size];
    for (auto i : util::range(dict_size))
        dict[i] = 0;

    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output.end<const u8>();
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end && input_ptr < input_end)
    {
        control <<= 1;
        if (!(control & 0x80))
            control = (*input_ptr++ << 8) | 0xFF;

        if ((control >> 15) & 1)
        {
            auto v = *input_ptr++;
            dict[dict_pos++] = *output_ptr++ = v;
            dict_pos %= dict_size;
        }
        else
        {
            if (input_ptr + 2 > input_end)
                break;
            u16 tmp = reinterpret_cast<const u16&>(*input_ptr);
            input_ptr += 2;

            u16 look_behind_pos = (tmp >> 4) % dict_size;
            u16 repetitions = (tmp & 0xF) + 3;
            while (repetitions-- && output_ptr < output_end)
            {
                dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
                look_behind_pos %= dict_size;
                dict_pos %= dict_size;
            }
        }
    }
    return output;
}

bool Lf2ImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid Lf2ImageDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());
    file.io.skip(4);
    auto width = file.io.read_u16_le();
    auto height = file.io.read_u16_le();
    auto size_orig = width * height;

    file.io.seek(0x16);
    auto color_count = file.io.read_u16_le();
    pix::Palette palette(color_count, file.io, pix::Format::BGR888);

    bstr output(width * height);
    auto data = file.io.read_to_eof();
    for (auto &c : data)
        c ^= 0xFF;
    data = custom_lzss_decompress(data, width * height);
    pix::Grid grid(width, height, data, palette);
    grid.flip();
    return grid;
}

static auto dummy = fmt::register_fmt<Lf2ImageDecoder>("leaf/lf2");
