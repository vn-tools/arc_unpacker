#include "fmt/real_live/nwa_audio_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::real_live;

namespace
{
    struct NwaHeader final
    {
        u16 channel_count;
        u16 bits_per_sample;
        u32 sample_rate;
        s32 compression_level;
        u32 block_count;
        u32 uncompressed_size;
        u32 compressed_size;
        u32 sample_count;
        u32 block_size;
        u32 rest_size;
    };

    class CustomBitReader final
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
        throw err::IoError("Reading bits beyond EOF");
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
    io::Stream &stream,
    const std::vector<u32> &offsets)
{
    size_t bytes_per_sample = header.bits_per_sample >> 3;
    size_t current_block_size = current_block != header.block_count - 1
        ? header.block_size * bytes_per_sample
        : header.rest_size * bytes_per_sample;
    auto output_size = current_block_size / bytes_per_sample;
    auto input_size = current_block != offsets.size() - 1
        ? offsets.at(current_block + 1) - offsets.at(current_block)
        : stream.size() - offsets.at(current_block);

    bstr output(output_size * 2);
    s16 *output_ptr = output.get<s16>();

    stream.seek(offsets.at(current_block));
    s16 d[2];
    for (auto i : util::range(header.channel_count))
    {
        if (header.bits_per_sample == 8)
            d[i] = stream.read_u8();
        else
            d[i] = stream.read_u16_le();
    }

    auto input = stream.read(
        input_size - bytes_per_sample * header.channel_count);
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

static bstr nwa_read_compressed(io::Stream &stream, const NwaHeader &header)
{
    if (header.compression_level < 0 || header.compression_level > 5)
        throw err::NotSupportedError("Unsupported compression level");

    if (header.channel_count != 1 && header.channel_count != 2)
        throw err::NotSupportedError("Unsupported channel count");

    if (header.bits_per_sample != 8 && header.bits_per_sample != 16)
        throw err::NotSupportedError("Unsupported bits per sample");

    if (!header.block_count)
        throw err::CorruptDataError("No blocks found");

    if (!header.compressed_size)
        throw err::CorruptDataError("No data found");

    if (header.uncompressed_size
        != header.sample_count * header.bits_per_sample / 8)
    {
        throw err::BadDataSizeError();
    }

    if (header.sample_count
        != (header.block_count - 1) * header.block_size + header.rest_size)
    {
        throw err::CorruptDataError("Bad sample count");
    }

    stream.skip(4);
    std::vector<u32> offsets;
    for (auto i : util::range(header.block_count))
        offsets.push_back(stream.read_u32_le());

    bstr output;
    for (auto i : util::range(header.block_count))
        output += decode_block(header, i, stream, offsets);
    return output;
}

static bstr nwa_read_uncompressed(io::Stream &stream, const NwaHeader &header)
{
    return stream.read(header.uncompressed_size);
}

bool NwaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.name.has_extension("nwa");
}

res::Audio NwaAudioDecoder::decode_impl(io::File &input_file) const
{
    // buffer the file in memory for performance
    io::MemoryStream stream(input_file.stream.read_to_eof());

    NwaHeader header;
    header.channel_count = stream.read_u16_le();
    header.bits_per_sample = stream.read_u16_le();
    header.sample_rate = stream.read_u32_le();
    header.compression_level = static_cast<s32>(stream.read_u32_le());
    stream.skip(4);
    header.block_count = stream.read_u32_le();
    header.uncompressed_size = stream.read_u32_le();
    header.compressed_size = stream.read_u32_le();
    header.sample_count = stream.read_u32_le();
    header.block_size = stream.read_u32_le();
    header.rest_size = stream.read_u32_le();

    bstr samples = header.compression_level == -1
        ? nwa_read_uncompressed(stream, header)
        : nwa_read_compressed(stream, header);

    res::Audio audio;
    audio.channel_count = header.channel_count;
    audio.bits_per_sample = header.bits_per_sample;
    audio.sample_rate = header.sample_rate;
    audio.samples = samples;
    return audio;
}

static auto dummy = fmt::register_fmt<NwaAudioDecoder>("real-live/nwa");
