#include "fmt/microsoft/wav_audio_decoder.h"
#include "log.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::microsoft;

static const bstr riff_magic = "RIFF"_b;
static const bstr wave_magic = "WAVE"_b;

bool WavAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(riff_magic.size()) == riff_magic
        && input_file.stream.seek(8).read(wave_magic.size()) == wave_magic;
}

std::unique_ptr<io::File> WavAudioDecoder::decode_impl(io::File &input_file) const
{
    sfx::Wave wave;
    input_file.stream.seek(12);
    while (!input_file.stream.eof())
    {
        const auto chunk_name = input_file.stream.read(4);
        const auto chunk_size = input_file.stream.read_u32_le();
        const auto chunk_start = input_file.stream.tell();
        if (chunk_name == "fmt\x20"_b)
        {
            wave.fmt.pcm_type = input_file.stream.read_u16_le();
            wave.fmt.channel_count = input_file.stream.read_u16_le();
            wave.fmt.sample_rate = input_file.stream.read_u32_le();
            const auto byte_rate = input_file.stream.read_u32_le();
            const auto block_align = input_file.stream.read_u16_le();
            wave.fmt.bits_per_sample = input_file.stream.read_u16_le();
            const auto chunk_pos = input_file.stream.tell() - chunk_start;
            if (chunk_pos < chunk_size)
            {
                const auto extra_data_size = input_file.stream.read_u16_le();
                wave.fmt.extra_data = input_file.stream.read(extra_data_size);
            }
            input_file.stream.seek(chunk_start + chunk_size);
        }
        else if (chunk_name == "data"_b)
        {
            wave.data.samples = input_file.stream.read(chunk_size);
        }
        else
        {
            Log.warn("Unknown chunk: %s\n", chunk_name.get<const char>());
            input_file.stream.skip(chunk_size);
        }
    }
    return util::file_from_wave(wave, input_file.name);
}

static auto dummy = fmt::register_fmt<WavAudioDecoder>("microsoft/wav");
