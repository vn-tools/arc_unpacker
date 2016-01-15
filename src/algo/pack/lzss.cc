#include "algo/pack/lzss.h"
#include <array>
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
    std::vector<u8> dict(1 << settings.position_bits, 0);
    auto dict_ptr
        = algo::make_cyclic_ptr(dict.data(), dict.size())
        + settings.initial_dictionary_pos;

    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    while (output_ptr.left())
    {
        if (bit_reader.get(1) > 0)
        {
            const auto b = bit_reader.get(8);
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            auto look_behind_pos = bit_reader.get(settings.position_bits);
            auto repetitions = bit_reader.get(settings.size_bits)
                + settings.min_match_size;
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + look_behind_pos;
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

bstr algo::pack::lzss_decompress(
    const bstr &input,
    const size_t output_size,
    const BytewiseLzssSettings &settings)
{
    std::array<u8, 0x1000> dict = {0};
    auto dict_ptr
        = algo::make_cyclic_ptr(dict.data(), dict.size())
        + settings.initial_dictionary_pos;

    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    auto input_ptr = algo::make_ptr(input);

    u16 control = 0;
    while (output_ptr.left())
    {
        control >>= 1;
        if (!(control & 0x100))
        {
            if (!input_ptr.left()) break;
            control = *input_ptr++ | 0xFF00;
        }
        if (control & 1)
        {
            if (!input_ptr.left()) break;
            const auto b = *input_ptr++;
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            if (input_ptr.left() < 2) break;
            const auto lo = *input_ptr++;
            const auto hi = *input_ptr++;
            const auto look_behind_pos = lo | ((hi & 0xF0) << 4);
            auto repetitions = (hi & 0xF) + 3;
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + look_behind_pos;
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
