#include "dec/twilight_frontier/pak2_audio_decoder.h"

using namespace au;
using namespace au::dec::twilight_frontier;

bool Pak2AudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("cv3");
}

res::Audio Pak2AudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto format = input_file.stream.read_le<u16>();
    const auto channel_count = input_file.stream.read_le<u16>();
    const auto sample_rate = input_file.stream.read_le<u32>();
    const auto byte_rate = input_file.stream.read_le<u32>();
    const auto block_align = input_file.stream.read_le<u16>();
    const auto bits_per_sample = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    const auto size = input_file.stream.read_le<u32>();
    const auto samples = input_file.stream.read(size);

    res::Audio audio;
    audio.channel_count = channel_count;
    audio.bits_per_sample = bits_per_sample;
    audio.sample_rate = sample_rate;
    audio.samples = samples;
    return audio;
}

static auto _ = dec::register_decoder<Pak2AudioDecoder>(
    "twilight-frontier/pak2-sfx");
