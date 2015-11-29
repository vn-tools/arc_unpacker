#include "fmt/twilight_frontier/pak2_audio_decoder.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

bool Pak2AudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("cv3");
}

res::Audio Pak2AudioDecoder::decode_impl(io::File &input_file) const
{
    const auto format = input_file.stream.read_u16_le();
    const auto channel_count = input_file.stream.read_u16_le();
    const auto sample_rate = input_file.stream.read_u32_le();
    const auto byte_rate = input_file.stream.read_u32_le();
    const auto block_align = input_file.stream.read_u16_le();
    const auto bits_per_sample = input_file.stream.read_u16_le();
    input_file.stream.skip(2);
    const auto size = input_file.stream.read_u32_le();
    const auto samples = input_file.stream.read(size);

    res::Audio audio;
    audio.channel_count = channel_count;
    audio.bits_per_sample = bits_per_sample;
    audio.sample_rate = sample_rate;
    audio.samples = samples;
    return audio;
}

static auto dummy
    = fmt::register_fmt<Pak2AudioDecoder>("twilight-frontier/pak2-sfx");
