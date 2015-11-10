#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace sfx {

    struct WaveFormatChunk final
    {
        WaveFormatChunk();
        u16 pcm_type;
        u16 channel_count;
        u32 sample_rate;
        u16 bits_per_sample;
        bstr extra_data;
    };

    struct WaveDataChunk final
    {
        bstr samples;
    };

    struct WaveSampleLoop final
    {
        u32 type;
        size_t start;
        size_t end;
        size_t fraction;
        size_t play_count;
    };

    struct WaveSamplerChunk final
    {
        u32 manufacturer;
        u32 product;
        u32 sample_period;
        u32 midi_unity_note;
        u32 midi_pitch_fraction;
        u32 smpte_format;
        u32 smpte_offset;
        std::vector<WaveSampleLoop> loops;
        bstr extra_data;
    };

    struct Wave final
    {
        WaveFormatChunk fmt;
        WaveDataChunk data;
        std::unique_ptr<WaveSamplerChunk> smpl;
    };

} }
