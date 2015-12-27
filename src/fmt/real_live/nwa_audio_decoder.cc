#include "fmt/real_live/nwa_audio_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::real_live;

namespace
{
    struct NwaHeader final
    {
        size_t channel_count;
        size_t bits_per_sample;
        size_t sample_rate;
        int compression_level;
        bool use_run_length;
        size_t block_count;
        size_t uncompressed_size;
        size_t compressed_size;
        size_t sample_count;
        size_t block_size;
        size_t rest_size;
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

CustomBitReader::CustomBitReader(const bstr &str) :
    str_ptr(str.get<const u8>()),
    str_end(str.end<const u8>()),
    shift(0),
    offset(0)
{
}

u32 CustomBitReader::get(size_t bits)
{
    if (shift > 8)
    {
        ++offset;
        shift -= 8;
    }
    if (str_ptr + offset + 2 >= str_end)
        throw err::IoError("Reading bits beyond EOF");
    const auto ret = *reinterpret_cast<const u16*>(&str_ptr[offset]) >> shift;
    shift += bits;
    return ret & ((1 << bits) - 1);
}

static bstr decode_block(
    const NwaHeader &header,
    size_t current_block,
    io::Stream &input_stream,
    const std::vector<u32> &offsets)
{
    const auto bytes_per_sample = header.bits_per_sample >> 3;
    const auto current_block_size = current_block != header.block_count - 1
        ? header.block_size * bytes_per_sample
        : header.rest_size * bytes_per_sample;
    const auto output_size = current_block_size / bytes_per_sample;
    const auto input_size = current_block != offsets.size() - 1
        ? offsets.at(current_block + 1) - offsets.at(current_block)
        : input_stream.size() - offsets.at(current_block);

    io::MemoryStream output_stream(output_size);

    input_stream.seek(offsets.at(current_block));
    s16 d[2];
    for (const auto i : algo::range(header.channel_count))
    {
        if (header.bits_per_sample == 8)
            d[i] = input_stream.read_u8();
        else
            d[i] = input_stream.read_u16_le();
    }

    const auto input = input_stream.read(
        input_size - bytes_per_sample * header.channel_count);
    CustomBitReader bit_reader(input);

    auto current_channel = 0;
    auto run_length = 0;
    for (const auto i : algo::range(output_size))
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
                    if (header.compression_level < 3)
                    {
                        bits -= header.compression_level;
                        shift += header.compression_level;
                    }
                    const auto mask1 = (1 << (bits - 1));
                    const auto mask2 = (1 << (bits - 1)) - 1;
                    const auto b = bit_reader.get(bits);
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
                const auto mask1 = (1 << (bits - 1));
                const auto mask2 = (1 << (bits - 1)) - 1;
                const auto b = bit_reader.get(bits);
                if (b & mask1)
                    d[current_channel] -= (b & mask2) << shift;
                else
                    d[current_channel] += (b & mask2) << shift;
            }
            else if (header.use_run_length)
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

        if (header.bits_per_sample == 8)
            output_stream.write_u8(d[current_channel]);
        else
            output_stream.write_u16_le(d[current_channel]);

        if (header.channel_count == 2)
            current_channel ^= 1;
    }

    return output_stream.seek(0).read_to_eof();
}

static bstr read_compressed_samples(
    io::Stream &input_stream, const NwaHeader &header)
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

    input_stream.skip(4);
    std::vector<u32> offsets;
    for (const auto i : algo::range(header.block_count))
        offsets.push_back(input_stream.read_u32_le());

    bstr output;
    for (const auto i : algo::range(header.block_count))
        output += decode_block(header, i, input_stream, offsets);
    return output;
}

static bstr read_uncompressed_samples(
    io::Stream &input_stream, const NwaHeader &header)
{
    return input_stream.read(header.uncompressed_size);
}

bool NwaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("nwa");
}

res::Audio NwaAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    // buffer the file in memory for performance
    io::MemoryStream input_stream(input_file.stream.seek(0).read_to_eof());

    NwaHeader header;
    header.channel_count = input_stream.read_u16_le();
    header.bits_per_sample = input_stream.read_u16_le();
    header.sample_rate = input_stream.read_u32_le();
    header.compression_level = static_cast<s32>(input_stream.read_u32_le());
    header.use_run_length = input_stream.read_u32_le() != 0;
    header.block_count = input_stream.read_u32_le();
    header.uncompressed_size = input_stream.read_u32_le();
    header.compressed_size = input_stream.read_u32_le();
    header.sample_count = input_stream.read_u32_le();
    header.block_size = input_stream.read_u32_le();
    header.rest_size = input_stream.read_u32_le();

    const auto samples = header.compression_level == -1
        ? read_uncompressed_samples(input_stream, header)
        : read_compressed_samples(input_stream, header);

    res::Audio audio;
    audio.channel_count = header.channel_count;
    audio.bits_per_sample = header.bits_per_sample;
    audio.sample_rate = header.sample_rate;
    audio.samples = samples;
    return audio;
}

static auto dummy = fmt::register_fmt<NwaAudioDecoder>("real-live/nwa");
