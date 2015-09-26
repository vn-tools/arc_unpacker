#include "util/audio.h"

using namespace au;
using namespace au::util;

struct Audio::Priv final
{
    size_t channel_count;
    size_t bytes_per_sample;
    size_t sample_rate;
    bstr samples;
};

Audio::Audio() : p(new Priv())
{
}

Audio::~Audio()
{
}

std::unique_ptr<Audio> Audio::from_samples(
    size_t channel_count,
    size_t bytes_per_sample,
    size_t sample_rate,
    const bstr &samples)
{
    std::unique_ptr<Audio> audio(new Audio);
    audio->p->channel_count = channel_count;
    audio->p->bytes_per_sample = bytes_per_sample;
    audio->p->sample_rate = sample_rate;
    audio->p->samples = samples;
    return audio;
}

std::unique_ptr<File> Audio::create_file(const std::string &name) const
{
    size_t block_align = p->channel_count * p->bytes_per_sample;
    size_t byte_rate = block_align * p->sample_rate;
    size_t bits_per_sample = p->bytes_per_sample * 8;
    size_t data_chunk_size = p->samples.size();

    std::unique_ptr<File> output_file(new File);

    output_file->io.write("RIFF"_b);
    output_file->io.write("\x00\x00\x00\x00"_b);
    output_file->io.write("WAVE"_b);
    output_file->io.write("fmt "_b);
    output_file->io.write_u32_le(16);
    output_file->io.write_u16_le(1);
    output_file->io.write_u16_le(p->channel_count);
    output_file->io.write_u32_le(p->sample_rate);
    output_file->io.write_u32_le(byte_rate);
    output_file->io.write_u16_le(block_align);
    output_file->io.write_u16_le(bits_per_sample);
    output_file->io.write("data"_b);
    output_file->io.write_u32_le(data_chunk_size);
    output_file->io.write(p->samples);
    output_file->io.seek(4);
    output_file->io.write_u32_le(output_file->io.size() - 8);

    output_file->name = name;
    output_file->change_extension("wav");
    return output_file;
}
