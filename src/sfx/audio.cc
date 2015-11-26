#include "sfx/audio.h"

using namespace au;
using namespace au::sfx;

Audio::Audio() :
    codec(1),
    extra_codec_headers(""_b),
    channel_count(1),
    bits_per_sample(16),
    sample_rate(44100),
    samples(""_b)
{
}
