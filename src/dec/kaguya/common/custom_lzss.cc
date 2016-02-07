#include "dec/kaguya/common/custom_lzss.h"
#include <array>
#include "algo/ptr.h"

using namespace au;
using namespace au::dec::kaguya;

bstr common::custom_lzss_decompress(
    const bstr &input, const size_t size_orig)
{
    std::array<u8, 256> dict = {0};
    u8 aux_byte = 0;

    bstr output(size_orig);
    auto output_ptr = algo::make_ptr(output);
    auto input_ptr = algo::make_ptr(input);
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size()) + 0xEF;
    bool flip = false;
    u8 repetitions = 0;
    u32 control = 0;
    while (output_ptr.left() && input_ptr.left())
    {
        control <<= 1;
        if (!(control & 0b10000000'0'00000000))
            control = *input_ptr++ | 0b11111111'0'00000000;
        if (control & 0b10000000)
        {
            const auto b = *input_ptr++;
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + *input_ptr++;
            if (flip)
            {
                repetitions = aux_byte >> 4;
            }
            else
            {
                aux_byte = *input_ptr++;
                repetitions = aux_byte & 0xF;
            }
            flip = !flip;
            repetitions += 2;
            while (repetitions-- && output_ptr.left())
            {
                const auto b = *source_ptr++;
                *output_ptr++ = b;
                *dict_ptr++ = b;
            }
        }
    }
    return output;
}
