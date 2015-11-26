#include "util/file_from_samples.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::util;

std::unique_ptr<io::File> util::file_from_samples(
    size_t channel_count,
    size_t bits_per_sample,
    size_t sample_rate,
    const bstr &samples,
    const io::path &name)
{
    sfx::Wave wave;
    wave.fmt.channel_count = channel_count;
    wave.fmt.sample_rate = sample_rate;
    wave.fmt.bits_per_sample = bits_per_sample;
    wave.data.samples = samples;
    return util::file_from_wave(wave, name);
}
