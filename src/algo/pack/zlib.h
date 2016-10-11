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

#include "algo/pack/compression_level.h"
#include "io/base_byte_stream.h"
#include "types.h"

namespace au {
namespace algo {
namespace pack {

    enum class ZlibKind : u8
    {
        RawDeflate = 0,
        PlainZlib  = 1, // == RawDeflate + 6 bytes of header+footer
        Gzip       = 2, // == PlainZlib + variable header data
    };

    bstr zlib_inflate(
        io::BaseByteStream &input_stream,
        const ZlibKind kind = ZlibKind::PlainZlib);

    bstr zlib_inflate(
        const bstr &input, const ZlibKind kind = ZlibKind::PlainZlib);

    bstr zlib_deflate(
        const bstr &input,
        const ZlibKind kind = ZlibKind::PlainZlib,
        const CompressionLevel = CompressionLevel::Best);


} } }
