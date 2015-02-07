#include <cassert>
#include "formats/sound.h"

struct Sound::Internals
{
    size_t channel_count;
    size_t bytes_per_sample;
    size_t sample_rate;
    std::string samples;
    size_t sample_count;
};

Sound::Sound() : internals(new Internals())
{
}

Sound::~Sound()
{
}

std::unique_ptr<Sound> Sound::from_samples(
    size_t channel_count,
    size_t bytes_per_sample,
    size_t sample_rate,
    const std::string &samples)
{
    std::unique_ptr<Sound> sound(new Sound);
    sound->internals->channel_count = channel_count;
    sound->internals->bytes_per_sample = bytes_per_sample;
    sound->internals->sample_rate = sample_rate;
    sound->internals->samples = samples;
    return sound;
}

void Sound::update_file(VirtualFile &file) const
{
    file.io.truncate(0);

    size_t block_align = internals->channel_count * internals->bytes_per_sample;
    size_t byte_rate = block_align * internals->sample_rate;
    size_t bits_per_sample = internals->bytes_per_sample * 8;

    file.io.write("RIFF", 4);
    file.io.write("\x00\x09\x00\x00", 4);
    file.io.write("WAVE", 4);
    file.io.write("fmt ", 4);
    file.io.write_u32_le(16);
    file.io.write_u16_le(1);
    file.io.write_u16_le(internals->channel_count);
    file.io.write_u32_le(internals->sample_rate);
    file.io.write_u32_le(byte_rate);
    file.io.write_u16_le(block_align);
    file.io.write_u16_le(bits_per_sample);
    file.io.write("data", 4);
    file.io.write_u32_le(internals->sample_count);
    file.io.write(internals->samples);
    file.io.seek(4);
    file.io.write_u32_le(file.io.size());

    file.change_extension("wav");
}
