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

#include "types.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    enum HuffmanFlags
    {
        Code = 0x80000000,
        Escape = 0x7FFFFFFF,
    };

    enum HuffmanNodes
    {
        Null = 0x8000,
        Root = 0x200,
    };

    struct HuffmanNode final
    {
        HuffmanNode();

        u16 weight;
        u16 parent;
        u32 code;
    };

    struct HuffmanTree final
    {
        HuffmanTree();
        void increase_occurrences(int entry);
        void recount_occurrences(int parent);
        void normalize(int entry);
        void add_new_entry(int new_code);
        void half_and_rebuild();

        HuffmanNode nodes[0x201];
        int sym_lookup[0x100];
        int escape;
        int tree_pointer;
    };

} } } }
