#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace sfx {

    struct LoopInfo final
    {
        size_t start;
        size_t end;
        size_t play_count; // 0 for infinite
    };

    struct Audio final
    {
        Audio();

        int codec;
        bstr extra_codec_headers; // = additional data in MS .wav fmt chunk

        size_t channel_count;
        size_t bits_per_sample;
        size_t sample_rate;
        std::vector<LoopInfo> loops;

        bstr samples;
    };

} }
