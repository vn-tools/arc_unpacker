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

#include <array>
#include "types.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    static const int prob_escape_code = -1;
    static const size_t prob_symbol_sorts = 0x101;
    static const size_t prob_total_limit = 0x2000;
    static const size_t prob_sub_sort_max = 0x80;

    struct CodeSymbol final
    {
        u16 occurrences;
        s16 symbol;
    };

    struct ProbModel final
    {
        ProbModel();
        void half_occurrence_count();
        void increase_symbol(const size_t index);
        void add_symbol(const s16 symbol);
        s16 find_symbol(const s16 symbol) const;

        u32 total_count;
        u32 symbol_sorts;
        std::array<CodeSymbol, prob_symbol_sorts> sym_table;
        std::array<CodeSymbol, prob_sub_sort_max> sub_model;
    };

} } } }
