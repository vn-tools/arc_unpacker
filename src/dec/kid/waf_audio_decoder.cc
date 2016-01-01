#include "dec/kid/waf_audio_decoder.h"

using namespace au;
using namespace au::dec::kid;

static const bstr magic = "WAF\x00\x00\x00"_b;

bool WafAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio WafAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>();

    input_file.stream.seek(6);

    res::Audio audio;
    audio.codec = 2;
    audio.channel_count = input_file.stream.read_u16_le();
    audio.sample_rate = input_file.stream.read_u32_le();
    const auto byte_rate = input_file.stream.read_u32_le();
    const auto block_align = input_file.stream.read_u16_le();
    audio.bits_per_sample = input_file.stream.read_u16_le();
    audio.extra_codec_headers = input_file.stream.read(32);
    const auto samples_size = input_file.stream.read_u32_le();
    audio.samples = input_file.stream.read(samples_size);
    return audio;
}

static auto _ = dec::register_decoder<WafAudioDecoder>("kid/waf");
