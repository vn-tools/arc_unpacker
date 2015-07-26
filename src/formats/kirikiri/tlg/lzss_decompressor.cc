#include "formats/kirikiri/tlg/lzss_decompressor.h"

using namespace au::fmt::kirikiri::tlg;

struct LzssDecompressor::Priv
{
    u8 dictionary[4096];
    size_t offset;

    Priv()
    {
        offset = 0;
        for (size_t i = 0; i < 4096; i++)
            dictionary[i] = 0;
    }
};

LzssDecompressor::LzssDecompressor() : p(new Priv)
{
}

LzssDecompressor::~LzssDecompressor()
{
}

void LzssDecompressor::init_dictionary(u8 dictionary[4096])
{
    for (size_t i = 0; i < 4096; i++)
        p->dictionary[i] = dictionary[i];
}

void LzssDecompressor::decompress(
    u8 *input, size_t input_size,
    u8 *output, size_t output_size)
{
    u8 *input_guardian = input + input_size;
    u8 *output_guardian = output + output_size;

    for (size_t i = 0; i < output_size; i++)
        output[i] = 0;

    int flags = 0;
    while (input < input_guardian)
    {
        flags >>= 1;
        if ((flags & 0x100) != 0x100)
        {
            if (input >= input_guardian)
                return;
            flags = *input++ | 0xff00;
        }

        if ((flags & 1) == 1)
        {
            if (input >= input_guardian)
                return;
            u8 x0 = *input++;
            if (input >= input_guardian)
                return;
            u8 x1 = *input++;
            size_t position = x0 | ((x1 & 0xf) << 8);
            size_t length = 3 + ((x1 & 0xf0) >> 4);
            if (length == 18)
            {
                if (input >= input_guardian)
                    return;
                length += *input++;
            }

            for (size_t j = 0; j < length; j++)
            {
                u8 c = p->dictionary[position];
                if (output >= output_guardian)
                    return;
                *output++ = c;
                p->dictionary[p->offset] = c;
                p->offset++;
                p->offset &= 0xfff;
                position++;
                position &= 0xfff;
            }
        }
        else
        {
            if (input >= input_guardian)
                return;
            u8 c = *input++;
            if (output >= output_guardian)
                return;
            *output++ = c;
            p->dictionary[p->offset] = c;
            p->offset++;
            p->offset &= 0xfff;
        }
    }
}
