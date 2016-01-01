#include "dec/lilim/dwv_audio_decoder.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::lilim;

static const bstr magic = "DW"_b;

bool DwvAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("dwv")
        && input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Audio DwvAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 2);
    const auto header_size = input_file.stream.read_u32_le();
    const auto samples_size = input_file.stream.read_u32_le();
    const auto header = input_file.stream.read(header_size);
    const auto samples = input_file.stream.read(samples_size);

    res::Audio audio;
    audio.samples = samples;

    io::MemoryStream header_stream(header);
    audio.codec = header_stream.read_u16_le();
    audio.channel_count = header_stream.read_u16_le();
    audio.sample_rate = header_stream.read_u32_le();
    const auto byte_rate = header_stream.read_u32_le();
    const auto block_align = header_stream.read_u16_le();
    audio.bits_per_sample = header_stream.read_u16_le();
    if (header_stream.tell() < header_stream.size())
    {
        const auto extra_data_size = header_stream.read_u16_le();
        audio.extra_codec_headers = header_stream.read(extra_data_size);
    }

    return audio;
}

static auto _ = dec::register_decoder<DwvAudioDecoder>("lilim/dwv");
