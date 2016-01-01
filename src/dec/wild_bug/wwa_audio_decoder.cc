#include "dec/wild_bug/wwa_audio_decoder.h"
#include "dec/wild_bug/wpx/decoder.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::wild_bug;

static const bstr magic = "WPX\x1AWAV\x00"_b;

bool WwaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio WwaAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);
    io::MemoryStream metadata_stream(decoder.read_plain_section(0x20));

    const auto pcm_type = metadata_stream.read_u16_le();
    const auto channel_count = metadata_stream.read_u16_le();
    const auto sample_rate = metadata_stream.read_u32_le();
    const auto byte_rate = metadata_stream.read_u32_le();
    const auto block_align = metadata_stream.read_u16_le();
    const auto bits_per_sample = metadata_stream.read_u16_le();

    const auto samples = decoder.read_compressed_section(0x21);

    res::Audio audio;
    audio.codec = pcm_type;
    audio.channel_count = channel_count;
    audio.sample_rate = sample_rate;
    audio.bits_per_sample = bits_per_sample;
    audio.samples = samples;
    return audio;
}

static auto _ = dec::register_decoder<WwaAudioDecoder>("wild-bug/wwa");
