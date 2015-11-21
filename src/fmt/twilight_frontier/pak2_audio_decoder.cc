#include "fmt/twilight_frontier/pak2_audio_decoder.h"
#include "util/file_from_samples.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

bool Pak2AudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("cv3");
}

std::unique_ptr<io::File> Pak2AudioDecoder::decode_impl(
    io::File &input_file) const
{
    auto format = input_file.stream.read_u16_le();
    auto channel_count = input_file.stream.read_u16_le();
    auto sample_rate = input_file.stream.read_u32_le();
    auto byte_rate = input_file.stream.read_u32_le();
    auto block_align = input_file.stream.read_u16_le();
    auto bits_per_sample = input_file.stream.read_u16_le();
    input_file.stream.skip(2);
    size_t size = input_file.stream.read_u32_le();

    return util::file_from_samples(
        channel_count,
        bits_per_sample,
        sample_rate,
        input_file.stream.read(size),
        input_file.name);
}

static auto dummy
    = fmt::register_fmt<Pak2AudioDecoder>("twilight-frontier/pak2-sfx");
