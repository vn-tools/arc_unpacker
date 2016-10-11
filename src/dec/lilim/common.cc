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

#include "dec/lilim/common.h"
#include "algo/pack/lzss.h"
#include "io/memory_byte_stream.h"

using namespace au;

bstr dec::lilim::sysd_decompress(const bstr &input)
{
    io::MemoryByteStream input_stream(input);
    const auto compressed = input_stream.read<u8>() != 0;
    const auto size_comp = input_stream.read_le<u32>();
    const auto size_orig = input_stream.read_le<u32>();
    auto data = input_stream.read(size_comp - 9);
    if (compressed)
        data = algo::pack::lzss_decompress(data, size_orig);
    return data;
}
