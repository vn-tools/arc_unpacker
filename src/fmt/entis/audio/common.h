#pragma once

#include "fmt/entis/common/enums.h"
#include "types.h"

namespace au {
namespace fmt {
namespace entis {
namespace audio {

    struct MioHeader final
    {
        u32 version;
        common::Transformation transformation;
        common::Architecture architecture;
        u32 channel_count;
        u32 sample_rate;
        u32 blockset_count;
        u32 subband_degree;
        u32 sample_count;
        u32 lapped_degree;
        u32 bits_per_sample;
    };

    struct MioChunk final
    {
        u8 version;
        bool initial;
        u32 sample_count;
        bstr data;
    };

    class AudioDecoderImpl
    {
    public:
        virtual ~AudioDecoderImpl();
        virtual bstr process_chunk(const MioChunk &) = 0;
    };

} } } }
