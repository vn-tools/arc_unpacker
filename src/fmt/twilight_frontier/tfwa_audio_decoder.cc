#include "fmt/twilight_frontier/tfwa_audio_decoder.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const bstr magic = "TFWA\x00"_b;

bool TfwaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

sfx::Audio TfwaAudioDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    auto format = input_file.stream.read_u16_le();
    auto channel_count = input_file.stream.read_u16_le();
    auto sample_rate = input_file.stream.read_u32_le();
    auto byte_rate = input_file.stream.read_u32_le();
    auto block_align = input_file.stream.read_u16_le();
    auto bits_per_sample = input_file.stream.read_u16_le();
    input_file.stream.skip(2);
    size_t size = input_file.stream.read_u32_le();

    sfx::Audio audio;
    audio.channel_count = channel_count;
    audio.bits_per_sample = bits_per_sample;
    audio.sample_rate = sample_rate;
    audio.samples = input_file.stream.read(size);
    return audio;
}

static auto dummy
    = fmt::register_fmt<TfwaAudioDecoder>("twilight-frontier/tfwa");
