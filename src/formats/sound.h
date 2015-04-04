#ifndef FORMATS_SOUND_H
#define FORMATS_SOUND_H
#include <memory>
#include "file.h"

class Sound final
{
public:
    ~Sound();

    static std::unique_ptr<Sound> from_samples(
        size_t channel_count,
        size_t bytes_per_sample,
        size_t sample_rate,
        const std::string &samples);

    std::unique_ptr<File> create_file(const std::string &base_file) const;

private:
    Sound();
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
