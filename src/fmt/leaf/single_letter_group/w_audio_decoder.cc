#include "fmt/leaf/single_letter_group/w_audio_decoder.h"
#include "err.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "bw\x20\x20"_b;

bool WAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.has_extension("w"))
        return false;
    const auto data_size = input_file.stream.seek(10).read_u32_le();
    return data_size + 18 == input_file.stream.size();
}

sfx::Wave WAudioDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    sfx::Wave audio;
    audio.fmt.channel_count = input_file.stream.read_u8();
    const auto block_align = input_file.stream.read_u8();
    audio.fmt.sample_rate = input_file.stream.read_u16_le();
    audio.fmt.bits_per_sample = input_file.stream.read_u16_le();
    const auto byte_rate = input_file.stream.read_u32_le();
    const auto samples_size = input_file.stream.read_u32_le();
    const auto loop_pos = input_file.stream.read_u32_le();
    if (loop_pos)
    {
        audio.smpl = std::make_unique<sfx::WaveSamplerChunk>();
        audio.smpl->loops.push_back(sfx::WaveSampleLoop
        {
            0,
            loop_pos,
            samples_size,
            0,
            0,
        });
    }
    audio.data.samples = input_file.stream.read(samples_size);
    return audio;
}

static auto dummy = fmt::register_fmt<WAudioDecoder>("leaf/w");
