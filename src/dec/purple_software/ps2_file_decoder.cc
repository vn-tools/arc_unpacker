#include "dec/purple_software/ps2_file_decoder.h"
#include "algo/binary.h"
#include "algo/cyclic_buffer.h"
#include "algo/range.h"
#include "ptr.h"

using namespace au;
using namespace au::dec::purple_software;

static const bstr magic = "PS2A"_b;

static void decrypt(bstr &data, const u32 key, const size_t shift)
{
    for (const auto i : algo::range(data.size()))
        data[i] = algo::rotr<u8>((data[i] - 0x7C) ^ key, shift);
}

static bstr custom_lzss_decompress(const bstr &input, const size_t size_orig)
{
    algo::CyclicBuffer<0x800> dict(0x7DF);
    bstr output(size_orig);
    auto output_ptr = make_ptr(output);
    auto input_ptr = make_ptr(input);
    u16 control = 1;
    while (output_ptr < output_ptr.end())
    {
        if (control == 1)
        {
            if (input_ptr >= input_ptr.end()) break;
            control = *input_ptr++ | 0x100;
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
            const u8 lo = *input_ptr++;
            const u8 hi = *input_ptr++;
            auto look_behind_pos = lo | (hi & 0xE0) << 3;
            auto repetitions = (hi & 0x1F) + 2;
            while (repetitions-- && output_ptr < output_ptr.end())
            {
                *output_ptr++ = dict[look_behind_pos++];
                dict << output_ptr[-1];
            }
        }
        control >>= 1;
    }
    return output;
}

bool Ps2FileDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    const auto size_comp = input_file.stream.seek(0x24).read_u32_le();
    const auto size_orig = input_file.stream.seek(0x28).read_u32_le();
    return size_comp != size_orig;
}

std::unique_ptr<io::File> Ps2FileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto tmp = input_file.stream.seek(12).read_u32_le();
    const auto key = (tmp >> 24) + (tmp >> 3);
    const auto shift = (tmp >> 20) % 5 + 1;

    const auto size_comp = input_file.stream.seek(0x24).read_u32_le();
    const auto size_orig = input_file.stream.seek(0x28).read_u32_le();
    auto data = input_file.stream.seek(0x30).read(size_comp);
    decrypt(data, key, shift);

    data = custom_lzss_decompress(data, size_orig);
    data = input_file.stream.seek(0).read(0x30) + data;
    *reinterpret_cast<u32*>(&data[0x24]) = size_orig;

    return std::make_unique<io::File>(input_file.path, data);
}

static auto _ = dec::register_decoder<Ps2FileDecoder>("purple-software/ps2");
