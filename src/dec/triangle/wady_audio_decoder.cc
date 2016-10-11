// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/triangle/wady_audio_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::triangle;

static const auto magic = "WADY"_b;

static s16 table[64] = {
    0,    2,    4,    6,    8,    10,   12,   15,
    18,   21,   24,   28,   32,   36,   40,   44,
    49,   54,   59,   64,   70,   76,   82,   88,
    95,   102,  109,  116,  124,  132,  140,  148,
    160,  170,  180,  190,  200,  210,  220,  230,
    240,  255,  270,  285,  300,  320,  340,  360,
    380,  400,  425,  450,  475,  500,  525,  550,
    580,  610,  650,  700,  750,  800,  900,  1000,
};

static bstr decode_audio(
    const bstr &input,
    const size_t sample_count,
    const size_t channel_count,
    const s16 mul)
{
    std::vector<s16> channel_holder(channel_count);
    bstr output(sample_count * channel_count * 2);
    auto input_ptr = input.get<u8>();
    auto output_ptr = output.get<s16>();
    for (const auto i : algo::range(sample_count))
    for (const auto j : algo::range(channel_count))
    {
        const auto tmp = *input_ptr++;
        if (tmp & 0x80)
            channel_holder[j] = tmp << 9;
        else if (tmp & 0x40)
            channel_holder[j] -= mul * table[tmp & 0x3F];
        else
            channel_holder[j] += mul * table[tmp & 0x3F];
        *output_ptr++ = channel_holder[j];
    }
    return output;
}

bool WadyAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio WadyAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>();

    input_file.stream.seek(magic.size());
    input_file.stream.skip(1);
    const auto mul = input_file.stream.read<u8>();
    const auto channel_count = input_file.stream.read_le<u16>();
    const auto sample_rate = input_file.stream.read_le<u32>();
    const auto data_size = input_file.stream.read_le<u32>();
    input_file.stream.seek(48);

    const auto input_size = input_file.stream.left();
    const auto sample_count = input_size / channel_count;

    const auto input = input_file.stream.read(input_size);
    const auto output = decode_audio(input, sample_count, channel_count, mul);

    res::Audio audio;
    audio.channel_count = channel_count;
    audio.bits_per_sample = 16;
    audio.sample_rate = sample_rate;
    audio.samples = output;
    return audio;
}

static auto _ = dec::register_decoder<WadyAudioDecoder>("triangle/wady");
