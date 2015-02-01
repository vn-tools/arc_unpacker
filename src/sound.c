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
    IO *io = io_create_empty();
    size_t block_align = sound->channel_count * sound->bytes_per_sample;
    size_t byte_rate = block_align * sound->sample_rate;
    size_t bits_per_sample = sound->bytes_per_sample * 8;

    io_write_string(io, "RIFF", 4);
    io_write_string(io, "\x00\x09\x00\x00", 4);
    io_write_string(io, "WAVE", 4);
    io_write_string(io, "fmt ", 4);
    io_write_u32_le(io, 16);
    io_write_u16_le(io, 1);
    io_write_u16_le(io, sound->channel_count);
    io_write_u32_le(io, sound->sample_rate);
    io_write_u32_le(io, byte_rate);
    io_write_u16_le(io, block_align);
    io_write_u16_le(io, bits_per_sample);
    io_write_string(io, "data", 4);
    io_write_u32_le(io, sound->sample_count);
    io_write_string(io, sound->samples, sound->sample_count);
    io_seek(io, 4);
    io_write_u32_le(io, io_size(io));

    io_seek(io, 0);
    char *data = (char*)malloc(io_size(io));
    assert_not_null(data);
    io_read_string(io, data, io_size(io));
    assert_that(vf_set_data(file, data, io_size(io)));
    io_destroy(io);
    free(data);

    vf_change_extension(file, "wav");
}
