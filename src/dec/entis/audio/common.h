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

#include "dec/entis/common/enums.h"
#include "types.h"

namespace au {
namespace dec {
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

    class BaseAudioDecoder
    {
    public:
        virtual ~BaseAudioDecoder() {}
        virtual bstr process_chunk(const MioChunk &chunk) = 0;
    };

} } } }
