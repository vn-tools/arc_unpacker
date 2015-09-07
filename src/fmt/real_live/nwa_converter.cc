// NWA music
//
// Company:   -
// Engine:    RealLive
// Extension: .nwa
// Archives:  -
//
// Known games:
// - [Hamham Soft] [071221] Imouto ni! Sukumizu Kisetara Nugasanai!
// - [Key] [041126] Kanon
// - [Key] [070928] Little Busters!
// - [Key] [080229] Clannad

#include "fmt/real_live/nwa_converter.h"
#include "io/buffered_io.h"
#include "util/range.h"
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

    class CustomBitReader
    {
    public:
        CustomBitReader(const bstr &str);
        u32 get(size_t bits);

    private:
        const u8 *str_ptr;
        const u8 *str_end;
        size_t shift;
        size_t offset;
    };
}

CustomBitReader::CustomBitReader(const bstr &str)
    : str_ptr(str.get<const u8>()),
        str_end(str.end<const u8>()),
        shift(0),
        offset(0)
{
}

u32 CustomBitReader::get(size_t bits)
{
    int ret;
    if (shift > 8)
    {
        ++offset;
        shift -= 8;
    }
    if (str_ptr + offset >= str_end)
        throw std::runtime_error("Reading bits beyond EOF");
    ret = *reinterpret_cast<const u16*>(&str_ptr[offset]) >> shift;
    shift += bits;
    return ret & ((1 << bits) - 1);
}

static bool use_run_length(const NwaHeader &header)
{
    return header.compression_level == 5 && header.channel_count == 2;
}

static bstr decode_block(
    const NwaHeader &header,
    size_t current_block,
    io::IO &io,
    const std::vector<u32> &offsets)
{
    size_t bytes_per_sample = header.bits_per_sample >> 3;
    size_t current_block_size = current_block != header.block_count - 1
        ? header.block_size * bytes_per_sample
        : header.rest_size * bytes_per_sample;
    auto output_size = current_block_size / bytes_per_sample;
    auto input_size = current_block != offsets.size() - 1
        ? offsets.at(current_block + 1) - offsets.at(current_block)
        : io.size() - offsets.at(current_block);

    bstr output;
    output.resize(output_size * 2);
    i16 *output_ptr = output.get<i16>();

    io.seek(offsets.at(current_block));
    i16 d[2];
    for (auto i : util::range(header.channel_count))
    {
        if (header.bits_per_sample == 8)
            d[i] = io.read_u8();
        else
            d[i] = io.read_u16_le();
    }

    auto input = io.read(input_size - bytes_per_sample * header.channel_count);
    CustomBitReader bit_reader(input);
    auto current_channel = 0;
    auto run_length = 0;
    for (auto i : util::range(output_size))
    {
        if (run_length)
        {
            run_length--;
        }
        else
        {
            int type = bit_reader.get(3);
            if (type == 7)
            {
                if (bit_reader.get(1))
                {
                    d[current_channel] = 0;
                }
                else
                {
                    int bits = 8, shift = 9;
                    if (header.compression_level >= 3)
                    {
                        bits -= header.compression_level;
                        shift += header.compression_level;
                    }
                    const int mask1 = (1 << (bits - 1));
                    const int mask2 = (1 << (bits - 1)) - 1;
                    int b = bit_reader.get(bits);
                    if (b & mask1)
                        d[current_channel] -= (b & mask2) << shift;
                    else
                        d[current_channel] += (b & mask2) << shift;
                }
            }
            else if (type > 0)
            {
                int bits, shift;
                if (header.compression_level >= 3)
                {
                    bits = 3 + header.compression_level;
                    shift = 1 + type;
                }
                else
                {
                    bits = 5 - header.compression_level;
                    shift = 2 + type + header.compression_level;
                }
                const int mask1 = (1 << (bits - 1));
                const int mask2 = (1 << (bits - 1)) - 1;
                int b = bit_reader.get(bits);
                if (b & mask1)
                    d[current_channel] -= (b & mask2) << shift;
                else
                    d[current_channel] += (b & mask2) << shift;
            }
            else if (use_run_length(header))
            {
                run_length = bit_reader.get(1);
                if (run_length == 1)
                {
                    run_length = bit_reader.get(2);
                    if (run_length == 3)
                        run_length = bit_reader.get(8);
                }
            }
        }

        output_ptr[i] = header.bits_per_sample == 8
            ? d[current_channel] * 0x100
            : d[current_channel];

        if (header.channel_count == 2)
            current_channel ^= 1;
    }

    return output;
}

static bstr nwa_read_compressed(io::IO &io, const NwaHeader &header)
{
    if (header.compression_level < 0 || header.compression_level > 5)
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
        != (header.block_count - 1) * header.block_size + header.rest_size)
    {
        throw std::runtime_error("Bad sample count");
    }

    io.skip(4);
    std::vector<u32> offsets;
    for (auto i : util::range(header.block_count))
        offsets.push_back(io.read_u32_le());

    bstr output;
    for (auto i : util::range(header.block_count))
        output += decode_block(header, i, io, offsets);
    return output;
}

static bstr nwa_read_uncompressed(io::IO &io, const NwaHeader &header)
{
    return io.read(header.uncompressed_size);
}

bool NwaConverter::is_recognized_internal(File &file) const
{
    return file.has_extension("nwa");
}

std::unique_ptr<File> NwaConverter::decode_internal(File &file) const
{
    // buffer the file in memory for performance
    io::BufferedIO io(file.io.read_to_eof());

    NwaHeader header;
    header.channel_count = io.read_u16_le();
    header.bits_per_sample = io.read_u16_le();
    header.sample_rate = io.read_u32_le();
    header.compression_level = static_cast<i32>(io.read_u32_le());
    io.skip(4);
    header.block_count = io.read_u32_le();
    header.uncompressed_size = io.read_u32_le();
    header.compressed_size = io.read_u32_le();
    header.sample_count = io.read_u32_le();
    header.block_size = io.read_u32_le();
    header.rest_size = io.read_u32_le();

    bstr samples = header.compression_level == -1
        ? nwa_read_uncompressed(io, header)
        : nwa_read_compressed(io, header);

    auto sound = util::Sound::from_samples(
        header.channel_count,
        header.bits_per_sample / 8,
        header.sample_rate,
        samples);
    return sound->create_file(file.name);
}

static auto dummy = fmt::Registry::add<NwaConverter>("rl/nwa");
