#ifndef FORMATS_SOUND_H
#define FORMATS_SOUND_H
#include "virtual_file.h"

typedef struct Sound Sound;

Sound *sound_create_from_samples(
    size_t channel_count,
    size_t bytes_per_sample,
    size_t sample_rate,
    char *samples,
    size_t sample_count);

void sound_destroy(Sound *sound);

void sound_update_file(const Sound *sound, VirtualFile *virtual_file);

#endif
