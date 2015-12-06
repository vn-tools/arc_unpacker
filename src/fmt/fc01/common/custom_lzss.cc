#include "fmt/fc01/common/custom_lzss.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

// Modified LZSS routine
// - repetition count and look behind pos differs
// - EOF is okay

using namespace au;
using namespace au::fmt::fc01;

bstr common::custom_lzss_decompress(const bstr &input, size_t output_size)
{
    io::MemoryStream stream(input);
    return common::custom_lzss_decompress(stream, output_size);
}

bstr common::custom_lzss_decompress(io::Stream &input, size_t output_size)
{
    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size] {};

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end && !input.eof())
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input.read_u8() | 0xFF00;

        if (control & 1)
        {
            const auto byte = input.read_u8();
            dict[dict_pos++] = *output_ptr++ = byte;
            dict_pos %= dict_size;
            continue;
        }

        const u16 tmp = input.read_u16_le();
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
