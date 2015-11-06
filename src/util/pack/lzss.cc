#include "util/pack/lzss.h"
#include "util/cyclic_buffer.h"
#include "util/range.h"

using namespace au;

bstr util::pack::lzss_decompress_bitwise(
    const bstr &input, size_t output_size, const LzssSettings &settings)
{
    io::BitReader bit_reader(input);
    return lzss_decompress_bitwise(bit_reader, output_size, settings);
}

bstr util::pack::lzss_decompress_bitwise(
    io::BitReader &bit_reader, size_t output_size, const LzssSettings &settings)
{
    bstr output;
    output.reserve(output_size);
    size_t dictionary_size = 1 << settings.position_bits;
    size_t dictionary_pos = settings.initial_dictionary_pos;
    auto dictionary = std::make_unique<u8[]>(dictionary_size);

    u8 *dictionary_ptr = dictionary.get();

    while (output.size() < output_size)
    {
        if (bit_reader.get(1) > 0)
        {
            u8 byte = bit_reader.get(8);
            output += byte;
            dictionary_ptr[dictionary_pos] = byte;
            dictionary_pos++;
            dictionary_pos %= dictionary_size;
        }
        else
        {
            unsigned int pos = bit_reader.get(settings.position_bits);
            unsigned int size = bit_reader.get(settings.size_bits);
            size += settings.min_match_size;
            for (auto i : util::range(size))
            {
                u8 byte = dictionary_ptr[pos];
                pos += 1;
                pos %= dictionary_size;
                dictionary_ptr[dictionary_pos] = byte;
                dictionary_pos++;
                dictionary_pos %= dictionary_size;
                output += byte;
                if (output.size() >= output_size)
                    break;
            }
        }
    }

    return output;
}

bstr util::pack::lzss_decompress_bytewise(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    util::CyclicBuffer<0x1000> dict(0xFEE);

    auto input_ptr = input.get<const u8>();
    auto output_ptr = output.get<u8>();
    const auto input_end = input.end<const u8>();
    const auto output_end = output.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end && input_ptr < input_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = *input_ptr++ | 0xFF00;

        if (control & 1)
        {
            *output_ptr++ = *input_ptr++;
            dict << output_ptr[-1];
        }
        else
        {
            u8 tmp1 = *input_ptr++;
            if (input_ptr >= input_end)
                break;
            u8 tmp2 = *input_ptr++;

            u16 look_behind_pos = ((tmp2 & 0xF0) << 4) | tmp1;
            u16 repetitions = (tmp2 & 0xF) + 3;
            while (repetitions-- && output_ptr < output_end)
            {
                *output_ptr++ = dict[look_behind_pos++];
                dict << output_ptr[-1];
            }
        }
    }
    return output;
}
