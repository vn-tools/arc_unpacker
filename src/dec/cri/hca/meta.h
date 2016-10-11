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
namespace dec {
namespace cri {
namespace hca {

    struct HcaChunk final
    {
        u16 version;
        size_t data_offset;
    };

    struct FmtChunk final
    {
        size_t channel_count;
        size_t sample_rate;
        size_t block_count;
    };

    struct CompChunk final
    {
        size_t block_size;
        u8 unk[8];
    };

    struct VbrChunk final
    {
        u16 unk[2];
    };

    struct AthChunk final
    {
        u16 type;
    };

    struct LoopChunk final
    {
        size_t start;
        size_t end;
        size_t repetitions;
        u16 unk;
    };

    struct CiphChunk final
    {
        u16 type;
    };

    struct RvaChunk final
    {
        float volume;
    };

    struct CommChunk final
    {
        bstr text;
    };

    struct Meta final
    {
        std::unique_ptr<HcaChunk> hca;
        std::unique_ptr<FmtChunk> fmt;
        std::unique_ptr<CompChunk> comp;
        std::unique_ptr<VbrChunk> vbr;
        std::unique_ptr<AthChunk> ath;
        std::unique_ptr<LoopChunk> loop;
        std::unique_ptr<CiphChunk> ciph;
        std::unique_ptr<RvaChunk> rva;
        std::unique_ptr<CommChunk> comm;
    };

    Meta read_meta(const bstr &input);

} } } }
