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

#include <memory>
#include "types.h"

namespace au {
namespace res {

    struct AudioLoopInfo final
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
        std::vector<AudioLoopInfo> loops;

        bstr samples;
    };

} }
