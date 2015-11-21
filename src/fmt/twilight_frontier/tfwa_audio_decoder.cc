#include "fmt/twilight_frontier/tfwa_audio_decoder.h"
#include "util/file_from_samples.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const bstr magic = "TFWA\x00"_b;

bool TfwaAudioDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> TfwaAudioDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());

    auto format = file.stream.read_u16_le();
    auto channel_count = file.stream.read_u16_le();
    auto sample_rate = file.stream.read_u32_le();
    auto byte_rate = file.stream.read_u32_le();
    auto block_align = file.stream.read_u16_le();
    auto bits_per_sample = file.stream.read_u16_le();
    file.stream.skip(2);
    size_t size = file.stream.read_u32_le();

    return util::file_from_samples(
        channel_count,
        bits_per_sample,
        sample_rate,
        file.stream.read(size),
        file.name);
}

static auto dummy
    = fmt::register_fmt<TfwaAudioDecoder>("twilight-frontier/tfwa");
