#include "fmt/ivory/wady_audio_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/file_from_samples.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::ivory;

static const bstr magic = "WADY"_b;

namespace
{
    enum Version
    {
        Version1,
        Version2,
    };
}

static Version detect_version(io::IO &io)
{
    auto version = Version::Version1;
    io.peek(io.tell(), [&]()
    {
        auto channels = io.read_u16_le();
        try
        {
            io.seek(0x30);
            if (channels == 2)
            {
                for (auto i : util::range(2))
                {
                    auto compressed_size = io.read_u32_le();
                    io.skip(compressed_size);
                }
            }
            else if (channels == 1)
            {
                io.skip(4);
                auto left = io.read_u32_le();
                io.skip(2);
                while (left--)
                {
                    if (!(io.read_u8() & 1))
                        io.read_u8();
                }
            }
            if (io.eof())
                version = Version::Version2;
        }
        catch (...)
        {
        }
    });
    return version;
}

static bstr decode_v1(
    io::IO &io, size_t sample_count, size_t channels, size_t block_align)
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

    u16 prev_sample[2] = { 0, 0 };
    io::BufferedIO tmp_io(io);
    while (!tmp_io.eof() && samples_ptr < samples_end)
    {
        for (auto i : util::range(channels))
        {
            u16 b = tmp_io.read_u8();
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

static bstr decode_v2(io::IO &io, size_t sample_count, size_t channels)
{
    static const u16 table1[] =
    {
        0x0000, 0x0004, 0x0008, 0x000C, 0x0013, 0x0018, 0x001E, 0x0026,
        0x002F, 0x003B, 0x004A, 0x005C, 0x0073, 0x0090, 0x00B4, 0x00E1,
        0x0119, 0x0160, 0x01B8, 0x0226, 0x02AF, 0x035B, 0x0431, 0x053E,
        0x068E, 0x0831, 0x0A3D, 0x0CCD, 0x1000, 0x1400, 0x1900, 0x1F40,
    };
    static const u32 table2[] = { 3, 4, 5, 6, 8, 16, 32, 256 };

    bstr samples(sample_count * 2 * channels);

    for (auto i : util::range(channels))
    {
        auto compressed_size = channels == 1
            ? io.size() - io.tell()
            : io.read_u32_le();

        io::BufferedIO tmp_io(io, compressed_size);
        tmp_io.skip(4);
        auto left = tmp_io.read_u32_le();
        s16 prev_sample = tmp_io.read_u16_le();

        auto samples_ptr = samples.get<u16>();
        auto samples_end = samples.end<u16>();
        samples_ptr[0] = prev_sample;
        samples_ptr += channels + i;

        while (left && samples_ptr < samples_end)
        {
            prev_sample = left == 300 ? 0 : prev_sample;
            u16 b = tmp_io.read_u8();

            if (b & 1)
            {
                if (b & 0x80)
                {
                    prev_sample = (b & ~1) << 9;
                }
                else
                {
                    u16 tmp = static_cast<s16>(b << 9) >> 15;
                    prev_sample += (tmp ^ table1[(b >> 1) & 0x1F]) - tmp;
                }
                *samples_ptr = prev_sample;
                samples_ptr += channels;
            }
            else
            {
                u32 tmp = (tmp_io.read_u8() << 8) | b;
                auto dividend = table2[(tmp >> 1) & 7];
                auto repetitions = table2[(tmp >> 1) & 7];
                s16 tmp2 = tmp & 0xFFF0;
                float sample = prev_sample;
                float delta = (tmp2 - sample) / static_cast<float>(dividend);
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

bool WadyAudioDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> WadyAudioDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());
    file.io.skip(2);

    auto version = detect_version(file.io);

    auto channels = file.io.read_u16_le();
    auto sample_rate = file.io.read_u32_le();

    auto sample_count = file.io.read_u32_le();
    auto channel_sample_count = file.io.read_u32_le();
    auto size_uncompressed = file.io.read_u32_le();
    if (channel_sample_count * channels != sample_count)
        throw err::CorruptDataError("Sample count mismatch");
    if (sample_count * 2 != size_uncompressed)
        throw err::CorruptDataError("Data size mismatch");
    file.io.skip(4 * 2 + 2);
    if (file.io.read_u16_le() != channels)
        throw err::CorruptDataError("Channel count mismatch");
    if (file.io.read_u32_le() != sample_rate)
        throw err::CorruptDataError("Sample rate mismatch");
    auto byte_rate = file.io.read_u32_le();
    auto block_align = file.io.read_u16_le();
    auto bits_per_sample = file.io.read_u16_le();

    file.io.seek(0x30);
    bstr samples;
    if (version == Version::Version1)
        samples = decode_v1(file.io, sample_count, channels, block_align);
    else if (version == Version::Version2)
        samples = decode_v2(file.io, sample_count, channels);
    else
        throw err::UnsupportedVersionError(version);

    return util::file_from_samples(
        channels, bits_per_sample / 8, sample_rate, samples, file.name);
}

static auto dummy = fmt::register_fmt<WadyAudioDecoder>("ivory/wady");
