#include <cassert>
#include "formats/sound.h"
#include "io.h"

struct Sound
{
    size_t channel_count;
    size_t bytes_per_sample;
    size_t sample_rate;
    char *samples;
    size_t sample_count;
};

Sound *sound_create_from_samples(
    size_t channel_count,
    size_t bytes_per_sample,
    size_t sample_rate,
    char *samples,
    size_t sample_count)
{
    Sound *sound = new Sound;
    assert(sound != nullptr);
    sound->channel_count = channel_count;
    sound->bytes_per_sample = bytes_per_sample;
    sound->sample_rate = sample_rate;
    sound->samples = samples;
    sound->sample_count = sample_count;
    return sound;
}

void sound_destroy(Sound *sound)
{
    assert(sound != nullptr);
    delete sound;
}

void sound_update_file(const Sound *sound, VirtualFile &file)
{
    assert(sound != nullptr);

    file.io.truncate(0);

    size_t block_align = sound->channel_count * sound->bytes_per_sample;
    size_t byte_rate = block_align * sound->sample_rate;
    size_t bits_per_sample = sound->bytes_per_sample * 8;

    file.io.write("RIFF", 4);
    file.io.write("\x00\x09\x00\x00", 4);
    file.io.write("WAVE", 4);
    file.io.write("fmt ", 4);
    file.io.write_u32_le(16);
    file.io.write_u16_le(1);
    file.io.write_u16_le(sound->channel_count);
    file.io.write_u32_le(sound->sample_rate);
    file.io.write_u32_le(byte_rate);
    file.io.write_u16_le(block_align);
    file.io.write_u16_le(bits_per_sample);
    file.io.write("data", 4);
    file.io.write_u32_le(sound->sample_count);
    file.io.write(sound->samples, sound->sample_count);
    file.io.seek(4);
    file.io.write_u32_le(file.io.size());

    file.change_extension("wav");
}
