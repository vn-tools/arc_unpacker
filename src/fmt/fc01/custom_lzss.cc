#include "fmt/fc01/custom_lzss.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/range.h"

// Modified LZSS routine
// - repetition count and look behind pos differs
// - EOF is okay

using namespace au;
using namespace au::fmt;

bstr fc01::custom_lzss_decompress(const bstr &input, size_t output_size)
{
    io::BufferedIO io(input);
    return fc01::custom_lzss_decompress(io, output_size);
}

bstr fc01::custom_lzss_decompress(io::IO &input_io, size_t output_size)
{
    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size] { };

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end && !input_io.eof())
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input_io.read_u8() | 0xFF00;

        if (control & 1)
        {
            auto byte = input_io.read_u8();
            dict[dict_pos++] = *output_ptr++ = byte;
            dict_pos %= dict_size;
            continue;
        }

        u16 tmp = input_io.read_u16_le();

        u32 look_behind_pos = tmp % dict_size;
        u16 repetitions = (tmp >> 12) + 3;
        while (repetitions-- && output_ptr < output_end)
        {
            dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
            look_behind_pos %= dict_size;
            dict_pos %= dict_size;
        }
    }
    return output;
}
