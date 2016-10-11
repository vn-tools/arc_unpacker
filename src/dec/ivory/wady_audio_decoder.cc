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

#include "dec/ivory/wady_audio_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::ivory;

static const bstr magic = "WADY"_b;

namespace
{
    enum Version
    {
        Version1,
        Version2,
    };
}

static Version detect_version(io::BaseByteStream &input_stream)
{
    auto version = Version::Version1;
    input_stream.peek(input_stream.pos(), [&]()
    {
        const auto channels = input_stream.read_le<u16>();
        try
        {
            input_stream.seek(0x30);
            if (channels == 2)
            {
                for (const auto i : algo::range(2))
                {
                    const auto size_comp = input_stream.read_le<u32>();
                    input_stream.skip(size_comp);
                }
            }
            else if (channels == 1)
            {
                input_stream.skip(4);
                auto left = input_stream.read_le<u32>();
                input_stream.skip(2);
                while (left--)
                {
                    if (!(input_stream.read<u8>() & 1))
                        input_stream.read<u8>();
                }
            }
            if (!input_stream.left())
                version = Version::Version2;
        }
        catch (...)
        {
        }
    });
    return version;
}

static bstr decode_v1(
    io::BaseByteStream &input_stream,
    const size_t sample_count,
    const size_t channels,
    const size_t block_align)
{
    static const u16 table[0x40] =
    {
        0x0000, 0x0002, 0x0004, 0x0006, 0x0008, 0x000A, 0x000C, 0x000F,
        0x0012, 0x0015, 0x0018, 0x001C, 0x0020, 0x0024, 0x0028, 0x002C,
        0x0031, 0x0036, 0x003B, 0x0040, 0x0046, 0x004C, 0x0052, 0x0058,
        0x005F, 0x0066, 0x006D, 0x0074, 0x007C, 0x0084, 0x008C, 0x0094,
        0x00A0, 0x00AA, 0x00B4, 0x00BE, 0x00C8, 0x00D2, 0x00DC, 0x00E6,
        0x00F0, 0x00FF, 0x010E, 0x011D, 0x012C, 0x0140, 0x0154, 0x0168,
        0x017C, 0x0190, 0x01A9, 0x01C2, 0x01DB, 0x01F4, 0x020D, 0x0226,
        0x0244, 0x0262, 0x028A, 0x02BC, 0x02EE, 0x0320, 0x0384, 0x03E8,
    };

    bstr samples(sample_count * 2 * channels);
    auto samples_ptr = samples.get<u16>();
    auto samples_end = samples.end<u16>();

    u16 prev_sample[2] = {0, 0};
    io::MemoryByteStream tmp_stream(input_stream);
    while (tmp_stream.left() && samples_ptr < samples_end)
    {
        for (const auto i : algo::range(channels))
        {
            const u16 b = tmp_stream.read<u8>();
            if (b & 0x80)
            {
                prev_sample[i] = b << 9;
            }
            else
            {
                u16 tmp = static_cast<s16>(b << 9) >> 15;
                tmp = (tmp ^ table[b & 0x3F]) - tmp;
                tmp *= block_align;
                prev_sample[i] += tmp;
            }
            *samples_ptr++ = prev_sample[i];
        }
    }

    return samples;
}

static bstr decode_v2(
    io::BaseByteStream &input_stream,
    const size_t sample_count,
    const size_t channels)
{
    static const u16 table1[] =
    {
        0x0000, 0x0004, 0x0008, 0x000C, 0x0013, 0x0018, 0x001E, 0x0026,
        0x002F, 0x003B, 0x004A, 0x005C, 0x0073, 0x0090, 0x00B4, 0x00E1,
        0x0119, 0x0160, 0x01B8, 0x0226, 0x02AF, 0x035B, 0x0431, 0x053E,
        0x068E, 0x0831, 0x0A3D, 0x0CCD, 0x1000, 0x1400, 0x1900, 0x1F40,
    };
    static const u32 table2[] = {3, 4, 5, 6, 8, 16, 32, 256};

    bstr samples(sample_count * 2 * channels);

    for (const auto i : algo::range(channels))
    {
        const auto size_comp = channels == 1
            ? input_stream.size() - input_stream.pos()
            : input_stream.read_le<u32>();

        io::MemoryByteStream tmp_stream(input_stream, size_comp);
        tmp_stream.skip(4);
        auto left = tmp_stream.read_le<u32>();
        s16 prev_sample = tmp_stream.read_le<u16>();

        auto samples_ptr = samples.get<u16>();
        auto samples_end = samples.end<u16>();
        samples_ptr[0] = prev_sample;
        samples_ptr += channels + i;

        while (left && samples_ptr < samples_end)
        {
            prev_sample = left == 300 ? 0 : prev_sample;
            const u16 b = tmp_stream.read<u8>();

            if (b & 1)
            {
                if (b & 0x80)
                {
                    prev_sample = (b & ~1) << 9;
                }
                else
                {
                    const u16 tmp = static_cast<s16>(b << 9) >> 15;
                    prev_sample += (tmp ^ table1[(b >> 1) & 0x1F]) - tmp;
                }
                *samples_ptr = prev_sample;
                samples_ptr += channels;
            }
            else
            {
                const u32 tmp = (tmp_stream.read<u8>() << 8) | b;
                const auto dividend = table2[(tmp >> 1) & 7];
                auto repetitions = table2[(tmp >> 1) & 7];
                const s16 tmp2 = tmp & 0xFFF0;
                float sample = prev_sample;
                const float delta
                    = (tmp2 - sample) / static_cast<float>(dividend);
                while (repetitions-- && samples_ptr < samples_end)
                {
                    sample += delta;
                    *samples_ptr = sample;
                    samples_ptr += channels;
                }
                prev_sample = tmp2;
            }
            --left;
        }
    }
    return samples;
}

bool WadyAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio WadyAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    input_file.stream.skip(2);

    const auto version = detect_version(input_file.stream);

    const auto channels = input_file.stream.read_le<u16>();
    const auto sample_rate = input_file.stream.read_le<u32>();

    const auto sample_count = input_file.stream.read_le<u32>();
    const auto channel_sample_count = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    if (channel_sample_count * channels != sample_count)
        throw err::CorruptDataError("Sample count mismatch");
    if (sample_count * 2 != size_orig)
        throw err::CorruptDataError("Data size mismatch");
    input_file.stream.skip(4 * 2 + 2);
    if (input_file.stream.read_le<u16>() != channels)
        throw err::CorruptDataError("Channel count mismatch");
    if (input_file.stream.read_le<u32>() != sample_rate)
        throw err::CorruptDataError("Sample rate mismatch");
    const auto byte_rate = input_file.stream.read_le<u32>();
    const auto block_align = input_file.stream.read_le<u16>();
    const auto bits_per_sample = input_file.stream.read_le<u16>();

    input_file.stream.seek(0x30);
    bstr samples;
    if (version == Version::Version1)
    {
        samples = decode_v1(
            input_file.stream, sample_count, channels, block_align);
    }
    else if (version == Version::Version2)
    {
        samples = decode_v2(input_file.stream, sample_count, channels);
    }
    else
    {
        throw err::UnsupportedVersionError(version);
    }

    res::Audio audio;
    audio.channel_count = channels;
    audio.bits_per_sample = bits_per_sample;
    audio.sample_rate = sample_rate;
    audio.samples = samples;
    return audio;
}

static auto _ = dec::register_decoder<WadyAudioDecoder>("ivory/wady");
