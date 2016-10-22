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

#include "dec/abogado/v_audio_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::abogado;

static const s16 table0[] = {
    0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
    0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F,
    0x0022, 0x0025, 0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042,
    0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076, 0x0082, 0x008F,
    0x009D, 0x00AD, 0x00BE, 0x00D1, 0x00E6, 0x00FD, 0x0117, 0x0133,
    0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292,
    0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583,
    0x0610, 0x06AB, 0x0756, 0x0812, 0x08E0, 0x09C3, 0x0ABD, 0x0BD0,
    0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
    0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B,
    0x3BB9, 0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462,
    0x7FFF,
};

static const int table1[] = {-1, -1, -1, -1, 2, 4, 6, 8};

template<typename T> static T clip(const T input, const T low, const T high)
{
    if (input < low)
        return low;
    if (input > high)
        return high;
    return input;
}

bool VAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto possible_sample_rate = input_file.stream.seek(0).read_le<u32>();
    return possible_sample_rate == 22050
        || possible_sample_rate == 44100;
}

static s16 transform_sample(u8 input, s16 &prev, int &acc)
{
    const auto input_high = input >> 3;
    const auto input_low = input & 7;
    const auto mul = table0[acc];
    acc = clip(acc + table1[input_low], 0, 0x58);
    s16 ret = clip(
        input_high
            ? prev - ((((input_low << 1) + 1) * mul) >> 3)
            : prev + ((((input_low << 1) + 1) * mul) >> 3),
        -32768,
        32767);
    prev = ret;
    return ret;
}

res::Audio VAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto sample_rate = input_file.stream.seek(0).read_le<u32>();
    const auto channel_count = input_file.stream.seek(4).read_le<u16>();
    const auto sample_count = input_file.stream.seek(188).read_le<u32>();
    const auto data = input_file.stream.seek(196).read_to_eof();

    io::MemoryByteStream sample_stream;
    if (channel_count == 2)
    {
        s16 lprev = 0, rprev = 0;
        int lacc = 0, racc = 0;
        for (const auto c : data)
        {
            sample_stream.write_le<s16>(transform_sample(c >> 4, lprev, lacc));
            sample_stream.write_le<s16>(transform_sample(c & 0xF, rprev, racc));
        }
    }
    else if (channel_count == 1)
    {
        s16 prev = 0;
        int acc = 0;
        for (const auto c : data)
        {
            sample_stream.write_le<s16>(transform_sample(c >> 4, prev, acc));
            sample_stream.write_le<s16>(transform_sample(c & 0xF, prev, acc));
        }
    }
    else
    {
        throw err::UnsupportedChannelCountError(channel_count);
    }

    res::Audio audio;
    audio.channel_count = channel_count;
    audio.bits_per_sample = 16;
    audio.sample_rate = sample_rate;
    audio.samples = sample_stream.seek(0).read_to_eof();
    return audio;
}

static auto _ = dec::register_decoder<VAudioDecoder>("abogado/v");
