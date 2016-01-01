#pragma once

#include <array>
#include "dec/wild_bug/wpx/common.h"

namespace au {
namespace dec {
namespace wild_bug {
namespace wpx {

    struct TranscriptionSpec final
    {
        size_t look_behind;
        size_t size;
    };

    struct ITranscriptionStrategy
    {
        virtual ~ITranscriptionStrategy() {}
        virtual TranscriptionSpec get_spec(DecoderContext &context) = 0;
    };

    struct TranscriptionStrategy1 final : ITranscriptionStrategy
    {
        TranscriptionStrategy1(
            const std::array<size_t, 8> &offsets, s8 quant_size);
        TranscriptionSpec get_spec(DecoderContext &context) override;
        const std::array<size_t, 8> &offsets;
        s8 quant_size;
    };

    struct TranscriptionStrategy2 final : ITranscriptionStrategy
    {
        TranscriptionStrategy2(
            const std::array<size_t, 8> &offsets, s8 quant_size);
        TranscriptionSpec get_spec(DecoderContext &context) override;
        const std::array<size_t, 8> &offsets;
        s8 quant_size;
    };

    struct TranscriptionStrategy3 final : ITranscriptionStrategy
    {
        TranscriptionSpec get_spec(DecoderContext &context) override;
    };

} } } }
