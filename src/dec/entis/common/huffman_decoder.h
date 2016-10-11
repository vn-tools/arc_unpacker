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

#include "dec/entis/common/base_decoder.h"
#include "dec/entis/common/huffman_tree.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    int get_huffman_code(io::BaseBitStream &bit_stream, HuffmanTree &tree);
    int get_huffman_size(io::BaseBitStream &bit_stream, HuffmanTree &tree);

    class HuffmanDecoder final : public BaseDecoder
    {
    public:
        HuffmanDecoder();
        ~HuffmanDecoder();

        void reset() override;
        void decode(u8 *output, const size_t output_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
