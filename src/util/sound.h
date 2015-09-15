#pragma once

#include <memory>
#include "file.h"

namespace au {
namespace util {

    class Sound final
    {
    public:
        ~Sound();

        static std::unique_ptr<Sound> from_samples(
            size_t channel_count,
            size_t bytes_per_sample,
            size_t sample_rate,
            const bstr &samples);

        std::unique_ptr<File> create_file(const std::string &base_file) const;

    private:
        Sound();

        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
