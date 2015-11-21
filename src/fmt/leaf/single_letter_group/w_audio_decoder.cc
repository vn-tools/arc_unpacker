#include "fmt/leaf/single_letter_group/w_audio_decoder.h"
#include "err.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "bw\x20\x20"_b;

bool WAudioDecoder::is_recognized_impl(File &input_file) const
{
    if (!input_file.has_extension("w"))
        return false;
    const auto data_size = input_file.stream.seek(10).read_u32_le();
    return data_size + 18 == input_file.stream.size();
}

std::unique_ptr<File> WAudioDecoder::decode_impl(File &input_file) const
{
    input_file.stream.seek(0);
    sfx::Wave wave;
    wave.fmt.channel_count = input_file.stream.read_u8();
    const auto block_align = input_file.stream.read_u8();
    wave.fmt.sample_rate = input_file.stream.read_u16_le();
    wave.fmt.bits_per_sample = input_file.stream.read_u16_le();
    const auto byte_rate = input_file.stream.read_u32_le();
    const auto samples_size = input_file.stream.read_u32_le();
    const auto loop_pos = input_file.stream.read_u32_le();
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
    const auto samples = input_file.stream.read(samples_size);

    if (block_align != wave.fmt.channel_count * wave.fmt.bits_per_sample / 8)
        throw err::CorruptDataError("Block align mismatch");
    if (byte_rate != block_align * wave.fmt.sample_rate)
        throw err::CorruptDataError("Byte rate mismatch");

    wave.data.samples = samples;
    return util::file_from_wave(wave, input_file.name);
}

static auto dummy = fmt::register_fmt<WAudioDecoder>("leaf/w");
