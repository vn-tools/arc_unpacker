#include "dec/microsoft/wav_audio_decoder.h"

using namespace au;
using namespace au::dec::microsoft;

static const bstr riff_magic = "RIFF"_b;
static const bstr wave_magic = "WAVE"_b;

bool WavAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(riff_magic.size()) == riff_magic
        && input_file.stream.seek(8).read(wave_magic.size()) == wave_magic;
}

res::Audio WavAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    res::Audio audio;
    input_file.stream.seek(12);
    while (!input_file.stream.eof())
    {
        const auto chunk_name = input_file.stream.read(4);
        const auto chunk_size = input_file.stream.read_le<u32>();
        const auto chunk_start = input_file.stream.pos();
        if (chunk_name == "fmt\x20"_b)
        {
            audio.codec = input_file.stream.read_le<u16>();
            audio.channel_count = input_file.stream.read_le<u16>();
            audio.sample_rate = input_file.stream.read_le<u32>();
            const auto byte_rate = input_file.stream.read_le<u32>();
            const auto block_align = input_file.stream.read_le<u16>();
            audio.bits_per_sample = input_file.stream.read_le<u16>();
            const auto chunk_pos = input_file.stream.pos() - chunk_start;
            if (chunk_pos < chunk_size)
            {
                const auto extra_data_size = input_file.stream.read_le<u16>();
                audio.extra_codec_headers
                    = input_file.stream.read(extra_data_size);
            }
            input_file.stream.seek(chunk_start + chunk_size);
        }
        else if (chunk_name == "data"_b)
        {
            audio.samples = input_file.stream.read(chunk_size);
        }
        else
        {
            logger.warn("Unknown chunk: %s\n", chunk_name.c_str());
            input_file.stream.skip(chunk_size);
        }
    }
    return audio;
}

static auto _ = dec::register_decoder<WavAudioDecoder>("microsoft/wav");
