#include "formats/kirikiri/tlg/lzss_decompressor.h"
using namespace Formats::Kirikiri::Tlg;

struct LzssDecompressor::Internals
{
    unsigned char dictionary[4096];
    unsigned int offset;

    Internals()
    {
        offset = 0;
        for (size_t i = 0; i < 4096; i ++)
            dictionary[i] = 0;
    }
};

LzssDecompressor::LzssDecompressor() : internals(new Internals)
{
}

LzssDecompressor::~LzssDecompressor()
{
}

void LzssDecompressor::init_dictionary(unsigned char dictionary[4096])
{
    for (size_t i = 0; i < 4096; i ++)
        internals->dictionary[i] = dictionary[i];
}

void LzssDecompressor::decompress(
    unsigned char *input,
    size_t input_size,
    unsigned char *output,
    size_t output_size)
{
    unsigned char *input_guardian = input + input_size;
    unsigned char *output_guardian = output + output_size;

    for (size_t i = 0; i < output_size; i ++)
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
            unsigned char x0 = *input++;
            if (input >= input_guardian)
                return;
            unsigned char x1 = *input++;
            size_t position = x0 | ((x1 & 0xf) << 8);
            size_t length = 3 + ((x1 & 0xf0) >> 4);
            if (length == 18)
            {
                if (input >= input_guardian)
                    return;
                length += *input++;
            }

            for (size_t j = 0; j < length; j ++)
            {
                unsigned char c = internals->dictionary[position];
                if (output >= output_guardian)
                    return;
                *output ++ = c;
                internals->dictionary[internals->offset] = c;
                internals->offset ++;
                internals->offset &= 0xfff;
                position ++;
                position &= 0xfff;
            }
        }
        else
        {
            if (input >= input_guardian)
                return;
            unsigned char c = *input ++;
            if (output >= output_guardian)
                return;
            *output ++ = c;
            internals->dictionary[internals->offset] = c;
            internals->offset ++;
            internals->offset &= 0xfff;
        }
    }
}
