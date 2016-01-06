#include "dec/leaf/common/custom_lzss.h"
#include "algo/cyclic_buffer.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::leaf;

// Modified LZSS routine
// - the bit shifts proceed in opposite direction
// - input is negated
bstr common::custom_lzss_decompress(const bstr &input, const size_t output_size)
{
    bstr output(output_size);
    algo::CyclicBuffer<u8, 0x1000> dict(0xFEE);
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
            dict << (*output_ptr++ = ~*input_ptr++);
        else
        {
            if (input_ptr + 2 > input_end)
                break;
            const u16 tmp = ~reinterpret_cast<const u16&>(*input_ptr);
            input_ptr += 2;
            u16 look_behind_pos = tmp >> 4;
            u16 repetitions = (tmp & 0xF) + 3;
            while (repetitions-- && output_ptr < output_end)
                dict << (*output_ptr++ = dict[look_behind_pos++]);
        }
    }
    return output;
}
