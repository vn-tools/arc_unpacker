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

#include "dec/wild_bug/wpx/common.h"

namespace au {
namespace dec {
namespace wild_bug {
namespace wpx {

    struct IRetrievalStrategy
    {
        IRetrievalStrategy(io::BaseBitStream &bit_stream) {}
        virtual ~IRetrievalStrategy() {}
        virtual u8 fetch_byte(DecoderContext &, const u8 *) = 0;
    };

    struct RetrievalStrategy1 final : IRetrievalStrategy
    {
        RetrievalStrategy1(io::BaseBitStream &bit_stream, s8 quant_size);
        u8 fetch_byte(DecoderContext &, const u8 *) override;
        std::vector<u8> dict;
        std::vector<u8> table;
        s8 quant_size;
    };

    struct RetrievalStrategy2 final : IRetrievalStrategy
    {
        RetrievalStrategy2(io::BaseBitStream &bit_stream);
        u8 fetch_byte(DecoderContext &, const u8 *) override;
        std::vector<u8> table;
    };

    struct RetrievalStrategy3 final : IRetrievalStrategy
    {
        RetrievalStrategy3(io::BaseBitStream &bit_stream);
        u8 fetch_byte(DecoderContext &, const u8 *) override;
    };

} } } }
