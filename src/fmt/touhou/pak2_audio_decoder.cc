#include "fmt/touhou/pak2_audio_decoder.h"
#include "util/file_from_samples.h"

using namespace au;
using namespace au::fmt::touhou;

bool Pak2AudioDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("cv3");
}

std::unique_ptr<File> Pak2AudioDecoder::decode_impl(File &file) const
{
    auto format = file.io.read_u16_le();
    auto channel_count = file.io.read_u16_le();
    auto sample_rate = file.io.read_u32_le();
    auto byte_rate = file.io.read_u32_le();
    auto block_align = file.io.read_u16_le();
    auto bits_per_sample = file.io.read_u16_le();
    file.io.skip(2);
    size_t size = file.io.read_u32_le();

    return util::file_from_samples(
        channel_count,
        bits_per_sample / 8,
        sample_rate,
        file.io.read(size),
        file.name);
}

static auto dummy = fmt::Registry::add<Pak2AudioDecoder>("th/pak2-sfx");
