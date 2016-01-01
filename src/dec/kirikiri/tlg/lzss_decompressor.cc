#include "dec/kirikiri/tlg/lzss_decompressor.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::kirikiri::tlg;

struct LzssDecompressor::Priv final
{
    Priv();

    u8 dictionary[4096];
    size_t offset;
};

LzssDecompressor::Priv::Priv()
{
    offset = 0;
    for (auto i : algo::range(4096))
        dictionary[i] = 0;
}

LzssDecompressor::LzssDecompressor() : p(new Priv)
{
}

LzssDecompressor::~LzssDecompressor()
{
}

void LzssDecompressor::init_dictionary(u8 dictionary[4096])
{
    for (auto i : algo::range(4096))
        p->dictionary[i] = dictionary[i];
}

bstr LzssDecompressor::decompress(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    u8 *output_ptr = output.get<u8>();
    const u8 *input_ptr = input.get<u8>();
    const u8 *input_end = input_ptr + input.size();
    const u8 *output_end = output_ptr + output.size();

    int flags = 0;
    while (input_ptr < input_end)
    {
        flags >>= 1;
        if ((flags & 0x100) != 0x100)
        {
            if (input_ptr >= input_end)
                return output;
            flags = *input_ptr++ | 0xFF00;
        }

        if ((flags & 1) == 1)
        {
            if (input_ptr >= input_end)
                return output;
            u8 x0 = *input_ptr++;
            if (input_ptr >= input_end)
                return output;
            u8 x1 = *input_ptr++;
            size_t position = x0 | ((x1 & 0xF) << 8);
            size_t size = 3 + ((x1 & 0xF0) >> 4);
            if (size == 18)
            {
                if (input_ptr >= input_end)
                    return output;
                size += *input_ptr++;
            }

            for (auto j : algo::range(size))
            {
                u8 c = p->dictionary[position];
                if (output_ptr >= output_end)
                    return output;
                *output_ptr++ = c;
                p->dictionary[p->offset] = c;
                p->offset++;
                p->offset &= 0xFFF;
                position++;
                position &= 0xFFF;
            }
        }
        else
        {
            if (input_ptr >= input_end)
                return output;
            u8 c = *input_ptr++;
            if (output_ptr >= output_end)
                return output;
            *output_ptr++ = c;
            p->dictionary[p->offset] = c;
            p->offset++;
            p->offset &= 0xFFF;
        }
    }

    return output;
}
