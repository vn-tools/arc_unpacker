#include "fmt/twilight_frontier/pak2_audio_decoder.h"
#include "util/file_from_samples.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

bool Pak2AudioDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("cv3");
}

std::unique_ptr<File> Pak2AudioDecoder::decode_impl(File &file) const
{
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
    = fmt::register_fmt<Pak2AudioDecoder>("twilight-frontier/pak2-sfx");
