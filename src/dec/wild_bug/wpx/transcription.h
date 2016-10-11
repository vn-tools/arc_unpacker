// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
