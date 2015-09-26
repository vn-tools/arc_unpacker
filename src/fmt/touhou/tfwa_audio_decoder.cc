#include "fmt/touhou/tfwa_audio_decoder.h"
#include "util/audio.h"

using namespace au;
using namespace au::fmt::touhou;

static const bstr magic = "TFWA\x00"_b;

bool TfwaAudioDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> TfwaAudioDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    auto format = file.io.read_u16_le();
    auto channel_count = file.io.read_u16_le();
    auto sample_rate = file.io.read_u32_le();
    auto byte_rate = file.io.read_u32_le();
    auto block_align = file.io.read_u16_le();
    auto bits_per_sample = file.io.read_u16_le();
    file.io.skip(2);
    size_t size = file.io.read_u32_le();

    auto audio = util::Audio::from_samples(
        channel_count,
        bits_per_sample / 8,
        sample_rate,
        file.io.read(size));
    return audio->create_file(file.name);
}

static auto dummy = fmt::Registry::add<TfwaAudioDecoder>("th/tfwa");
