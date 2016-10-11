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

#include "dec/cri/hca/ath_table.h"
#include "io/base_bit_stream.h"

namespace au {
namespace dec {
namespace cri {
namespace hca {

    class ChannelDecoder final
    {
    public:
        ChannelDecoder(const int type, const int idx, const int count);

        void decode1(
            io::BaseBitStream &bit_stream,
            const unsigned int a,
            const int b,
            const AthTable &ath_table);

        void decode2(io::BaseBitStream &bit_stream);

        void decode3(
            const unsigned int a,
            const unsigned int b,
            const unsigned int c,
            const unsigned int d);

        void decode4(
            const int index,
            const unsigned int a,
            const unsigned int b,
            const unsigned int c,
            ChannelDecoder &next_decoder);

        void decode5(const int index);

        f32 wave[8][128];

    private:
        int type;
        unsigned int count;
        u8 scale[128];
        u8 value[128];
        u8 value2[8];
        u8 *value3;
        f32 block[128];
        f32 base[128];
        f32 wav1[128];
        f32 wav2[128];
        f32 wav3[128];
    };

} } } }
