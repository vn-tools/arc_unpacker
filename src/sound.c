#include <stdlib.h>
#include "assert_ex.h"
#include "io.h"
#include "sound.h"

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
    Sound *sound = (Sound*)malloc(sizeof(Sound));
    assert_not_null(sound);
    sound->channel_count = channel_count;
    sound->bytes_per_sample = bytes_per_sample;
    sound->sample_rate = sample_rate;
    sound->samples = samples;
    sound->sample_count = sample_count;
    return sound;
}

void sound_destroy(Sound *sound)
{
    assert_not_null(sound);
    free(sound);
}

void sound_update_file(const Sound *sound, VirtualFile *file)
{
    assert_not_null(sound);
    assert_not_null(file);

    assert_that(io_truncate(file->io, 0));
    size_t block_align = sound->channel_count * sound->bytes_per_sample;
    size_t byte_rate = block_align * sound->sample_rate;
    size_t bits_per_sample = sound->bytes_per_sample * 8;

    io_write_string(file->io, "RIFF", 4);
    io_write_string(file->io, "\x00\x09\x00\x00", 4);
    io_write_string(file->io, "WAVE", 4);
    io_write_string(file->io, "fmt ", 4);
    io_write_u32_le(file->io, 16);
    io_write_u16_le(file->io, 1);
    io_write_u16_le(file->io, sound->channel_count);
    io_write_u32_le(file->io, sound->sample_rate);
    io_write_u32_le(file->io, byte_rate);
    io_write_u16_le(file->io, block_align);
    io_write_u16_le(file->io, bits_per_sample);
    io_write_string(file->io, "data", 4);
    io_write_u32_le(file->io, sound->sample_count);
    io_write_string(file->io, sound->samples, sound->sample_count);
    io_seek(file->io, 4);
    io_write_u32_le(file->io, io_size(file->io));

    vf_change_extension(file, "wav");
}
