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

#include "io/base_bit_stream.h"

namespace au {
namespace algo {
namespace pack {

    struct HuffmanTree final
    {
        HuffmanTree(const bstr &data);
        HuffmanTree(io::BaseBitStream &input_stream);

        int size;
        u16 root;
        u16 nodes[2][512];
    };

    bstr decode_huffman(
        const HuffmanTree &huffman_tree,
        const bstr &input,
        const size_t target_size);

} } }
