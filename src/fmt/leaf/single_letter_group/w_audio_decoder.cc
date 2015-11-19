#include "fmt/leaf/single_letter_group/w_audio_decoder.h"
#include "err.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "bw\x20\x20"_b;

bool WAudioDecoder::is_recognized_impl(File &file) const
{
    if (!file.has_extension("w"))
        return false;
    return file.io.seek(10).read_u32_le() + 18 == file.io.size();
}

std::unique_ptr<File> WAudioDecoder::decode_impl(File &file) const
{
    file.io.seek(0);
    sfx::Wave wave;
    wave.fmt.channel_count = file.io.read_u8();
    const auto block_align = file.io.read_u8();
    wave.fmt.sample_rate = file.io.read_u16_le();
    wave.fmt.bits_per_sample = file.io.read_u16_le();
    const auto byte_rate = file.io.read_u32_le();
    const auto samples_size = file.io.read_u32_le();
    const auto loop_pos = file.io.read_u32_le();
    if (loop_pos)
    {
        wave.smpl = std::make_unique<sfx::WaveSamplerChunk>();
        wave.smpl->loops.push_back(sfx::WaveSampleLoop
        {
            0,
            loop_pos,
            samples_size,
            0,
            0,
        });
    }
    const auto samples = file.io.read(samples_size);

    if (block_align != wave.fmt.channel_count * wave.fmt.bits_per_sample / 8)
        throw err::CorruptDataError("Block align mismatch");
    if (byte_rate != block_align * wave.fmt.sample_rate)
        throw err::CorruptDataError("Byte rate mismatch");

    wave.data.samples = samples;
    return util::file_from_wave(wave, file.name);
}

static auto dummy = fmt::register_fmt<WAudioDecoder>("leaf/w");
