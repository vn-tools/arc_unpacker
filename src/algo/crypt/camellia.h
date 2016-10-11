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

#include <vector>
#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    class Camellia final
    {
    public:
        Camellia(
            const std::vector<u32> &key,
            const size_t grand_rounds = 3);

        void encrypt_block_128(
            const size_t block_offset,
            const u32 input[4],
            u32 output[4]) const;

        void decrypt_block_128(
            const size_t block_offset,
            const u32 input[4],
            u32 output[4]) const;

    private:
        const std::vector<u32> key;
        const size_t grand_rounds;
    };

} } }
