#include "fmt/kid/waf_audio_decoder.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "WAF\x00\x00\x00"_b;

bool WafAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> WafAudioDecoder::decode_impl(
    io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>();

    input_file.stream.seek(6);

    sfx::Wave audio;
    audio.fmt.pcm_type = 2;
    audio.fmt.channel_count = input_file.stream.read_u16_le();
    audio.fmt.sample_rate = input_file.stream.read_u32_le();
    const auto byte_rate = input_file.stream.read_u32_le();
    const auto block_align = input_file.stream.read_u16_le();
    audio.fmt.bits_per_sample = input_file.stream.read_u16_le();
    audio.fmt.extra_data = input_file.stream.read(32);
    const auto samples_size = input_file.stream.read_u32_le();
    audio.data.samples = input_file.stream.read(samples_size);
    return util::file_from_wave(audio, input_file.name);
}

static auto dummy = fmt::register_fmt<WafAudioDecoder>("kid/waf");
