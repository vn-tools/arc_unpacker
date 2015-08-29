// NWA music
//
// Company:   -
// Engine:    RealLive
// Extension: .nwa
// Archives:  -
//
// Known games:
// - Clannad
// - Little Busters
// - Kanon

#include "fmt/real_live/nwa_converter.h"
#include "util/sound.h"

using namespace au;
using namespace au::fmt::real_live;

namespace
{
    struct NwaHeader
    {
        u16 channel_count;
        u16 bits_per_sample;
        u32 sample_rate;
        i32 compression_level;
        u32 block_count;
        u32 uncompressed_size;
        u32 compressed_size;
        u32 sample_count;
        u32 block_size;
        u32 rest_size;
    };
}

static void nwa_validate_header(const NwaHeader &header)
{
    if (header.compression_level >= 0 || header.compression_level > 5)
        throw std::runtime_error("Unsupported compression level");

    if (header.channel_count != 1 && header.channel_count != 2)
        throw std::runtime_error("Unsupported channel count");

    if (header.bits_per_sample != 8 && header.bits_per_sample != 16)
        throw std::runtime_error("Unsupported bits per sample");

    if (!header.block_count)
        throw std::runtime_error("No blocks found");

    if (!header.compressed_size)
        throw std::runtime_error("No data found");

    if (header.uncompressed_size
        != header.sample_count * header.bits_per_sample / 8)
    {
        throw std::runtime_error("Bad data size");
    }

    if (header.sample_count
        != (header.block_count-1) * header.block_size + header.rest_size)
    {
        throw std::runtime_error("Bad sample count");
    }
}

static bstr nwa_read_uncompressed(io::IO &io, const NwaHeader &header)
{
    return io.read(header.block_size * header.channel_count);
}

static bstr nwa_read_compressed(io::IO &, const NwaHeader &)
{
    throw std::runtime_error("Reading compressed streams is not supported");
}

bool NwaConverter::is_recognized_internal(File &file) const
{
    return file.has_extension("nwa");
}

std::unique_ptr<File> NwaConverter::decode_internal(File &file) const
{
    NwaHeader header;
    header.channel_count = file.io.read_u16_le();
    header.bits_per_sample = file.io.read_u16_le();
    header.sample_rate = file.io.read_u32_le();
    header.compression_level = static_cast<i32>(file.io.read_u32_le());
    header.block_count = file.io.read_u32_le();
    header.uncompressed_size = file.io.read_u32_le();
    header.compressed_size = file.io.read_u32_le();
    header.sample_count = file.io.read_u32_le();
    header.block_size = file.io.read_u32_le();
    header.rest_size = file.io.read_u32_le();

    bstr samples;

    if (header.compression_level == -1
        || header.block_count == 0
        || header.compressed_size == 0
        || header.block_size == 0
        || header.rest_size == 0)
    {
        samples = nwa_read_uncompressed(file.io, header);
    }
    else
    {
        nwa_validate_header(header);
        samples = nwa_read_compressed(file.io, header);
    }

    auto sound = util::Sound::from_samples(
        header.channel_count,
        header.bits_per_sample / 8,
        header.sample_rate,
        samples);
    return sound->create_file(file.name);
}

static auto dummy = fmt::Registry::add<NwaConverter>("rl/nwa");
