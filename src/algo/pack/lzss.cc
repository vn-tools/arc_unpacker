#include "algo/pack/lzss.h"
#include "algo/cyclic_buffer.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "io/msb_bit_reader.h"

using namespace au;

algo::pack::BytewiseLzssSettings::BytewiseLzssSettings()
    : initial_dictionary_pos(0xFEE)
{
}

bstr algo::pack::lzss_decompress(
    const bstr &input,
    const size_t output_size,
    const BitwiseLzssSettings &settings)
{
    io::MsbBitReader bit_reader(input);
    return lzss_decompress(bit_reader, output_size, settings);
}

bstr algo::pack::lzss_decompress(
    io::IBitReader &bit_reader,
    const size_t output_size,
    const BitwiseLzssSettings &settings)
{
    bstr output(output_size);
    const auto dict_size = 1 << settings.position_bits;
    auto dict_pos = settings.initial_dictionary_pos;
    auto dict = std::make_unique<u8[]>(dict_size);
    auto output_ptr = algo::make_ptr(output);
    auto dict_ptr = dict.get();
    while (output_ptr < output.end())
    {
        if (bit_reader.get(1) > 0)
        {
            *output_ptr++ = bit_reader.get(8);
            dict_ptr[dict_pos] = output_ptr[-1];
            dict_pos++;
            dict_pos %= dict_size;
        }
        else
        {
            auto look_behind_pos = bit_reader.get(settings.position_bits);
            auto repetitions = bit_reader.get(settings.size_bits)
                + settings.min_match_size;
            while (repetitions-- && output_ptr < output.end())
            {
                *output_ptr++ = dict_ptr[look_behind_pos];
                look_behind_pos++;
                look_behind_pos %= dict_size;
                dict_ptr[dict_pos] = output_ptr[-1];
                dict_pos++;
                dict_pos %= dict_size;
            }
        }
    }
    return output;
}

bstr algo::pack::lzss_decompress(
    const bstr &input,
    const size_t output_size,
    const BytewiseLzssSettings &settings)
{
    algo::CyclicBuffer<u8, 0x1000> dict(settings.initial_dictionary_pos);
    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    auto input_ptr = algo::make_ptr(input);
    u16 control = 0;
    while (output_ptr < output_ptr.end())
    {
        control >>= 1;
        if (!(control & 0x100))
        {
            if (input_ptr >= input_ptr.end()) break;
            control = *input_ptr++ | 0xFF00;
        }
        if (control & 1)
        {
            if (input_ptr >= input_ptr.end()) break;
            *output_ptr++ = *input_ptr++;
            dict << output_ptr[-1];
        }
        else
        {
            if (input_ptr + 2 > input_ptr.end()) break;
            const auto lo = *input_ptr++;
            const auto hi = *input_ptr++;
            auto look_behind_pos = lo | ((hi & 0xF0) << 4);
            auto repetitions = (hi & 0xF) + 3;
            while (repetitions-- && output_ptr < output_ptr.end())
            {
                *output_ptr++ = dict[look_behind_pos++];
                dict << output_ptr[-1];
            }
        }
    }
    return output;
}
