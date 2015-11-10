#include "fmt/leaf/common/custom_lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

// Modified LZSS routine
// - the bit shifts proceed in opposite direction
// - input is negated
bstr common::custom_lzss_decompress(const bstr &input, size_t output_size)
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
            control = (~*input_ptr++ << 8) | 0xFF;

        if ((control >> 15) & 1)
        {
            auto v = ~*input_ptr++;
            dict[dict_pos++] = *output_ptr++ = v;
            dict_pos %= dict_size;
        }
        else
        {
            if (input_ptr + 2 > input_end)
                break;
            u16 tmp = ~reinterpret_cast<const u16&>(*input_ptr);
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
