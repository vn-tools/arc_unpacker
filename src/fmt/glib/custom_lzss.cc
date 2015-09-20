#include "fmt/glib/custom_lzss.h"
#include "io/buffered_io.h"
#include "util/range.h"

// Modified LZSS routines (repetition count is negated)

using namespace au;
using namespace au::fmt;

bstr glib::custom_lzss_decompress(const bstr &input, size_t output_size)
{
    io::BufferedIO io(input);
    return glib::custom_lzss_decompress(io, output_size);
}

bstr glib::custom_lzss_decompress(io::IO &input_io, size_t output_size)
{
    bstr output(output_size);

    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size] { };

    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end)
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

        u32 tmp1 = input_io.read_u8();
        u32 tmp2 = input_io.read_u8();

        u32 look_behind_pos = (((tmp2 & 0xF0) << 4) | tmp1) % dict_size;
        u16 repetitions = (~tmp2 & 0xF) + 3;
        while (repetitions-- && output_ptr < output_end)
        {
            dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
            look_behind_pos %= dict_size;
            dict_pos %= dict_size;
        }
    }
    return output;
}
