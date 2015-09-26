#pragma once

#include <memory>
#include "file.h"

namespace au {
namespace util {

    class Audio final
    {
    public:
        ~Audio();

        static std::unique_ptr<Audio> from_samples(
            size_t channel_count,
            size_t bytes_per_sample,
            size_t sample_rate,
            const bstr &samples);

        std::unique_ptr<File> create_file(const std::string &base_file) const;

    private:
        Audio();

        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
