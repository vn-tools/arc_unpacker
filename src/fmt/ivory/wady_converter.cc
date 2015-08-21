// WADY music file
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: -
// Archives:  MBL

#include "fmt/ivory/wady_converter.h"
#include "io/buffered_io.h"
#include "util/range.h"
#include "util/require.h"
#include "util/sound.h"

using namespace au;
using namespace au::fmt::ivory;

static const bstr magic = "WADY"_b;

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
            for (auto i : util::range(channels))
            {
                auto channel_size = io.read_u32_le();
                io.skip(channel_size);
            };
            if (io.eof())
                version = Version::Version2;
        }
        catch (...)
        {
        }
    });
    return version;
}

bool WadyConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> WadyConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    file.io.skip(2);

    auto version = detect_version(file.io);
    if (version == Version::Version2)
        throw std::runtime_error("Only uncompressed audio is supported");

    auto channels = file.io.read_u16_le();
    auto sample_rate = file.io.read_u32_le();
    file.io.skip(4 * 2);
    auto sample_count = file.io.read_u32_le();
    file.io.skip(4 * 2 + 2);
    util::require(file.io.read_u16_le() == channels);
    util::require(file.io.read_u32_le() == sample_rate);
    auto bytes_per_sample = file.io.read_u32_le();
    auto block_align = file.io.read_u16_le();
    auto bits_per_sample = file.io.read_u16_le();

    file.io.seek(0x30);
    io::BufferedIO tmp_io(file.io);
    bstr samples;
    samples.resize(tmp_io.size() * 2);
    auto samples_ptr = samples.get<u16>();

    u16 prev[2] = { 0, 0 };
    while (!tmp_io.eof())
    {
        for (auto i : util::range(channels))
        {
            u16 c = tmp_io.read_u8();
            if (c & 0x80)
            {
                c <<= 9;
            }
            else
            {
                u16 tmp = static_cast<i16>(c << 9) >> 15;
                tmp = (tmp ^ table[c & 0x3F]) - tmp;
                tmp *= block_align;
                c = prev[i] + tmp;
            }
            *samples_ptr++ = c;
            prev[i] = c;
        }
    }

    auto sound = util::Sound::from_samples(
        channels, bits_per_sample / 8, sample_rate, samples);
    return sound->create_file(file.name);
}
