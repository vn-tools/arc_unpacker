#include "sfx/wave.h"

using namespace au;
using namespace au::sfx;

WaveFormatChunk::WaveFormatChunk() :
    pcm_type(1),
    channel_count(1),
    sample_rate(44100),
    bits_per_sample(16),
    extra_data(""_b)
{
}
