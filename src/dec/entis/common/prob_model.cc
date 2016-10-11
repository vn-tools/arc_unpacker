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

#include "dec/entis/common/prob_model.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::entis::common;

ProbModel::ProbModel()
{
    total_count = sym_table.size();
    symbol_sorts = sym_table.size();
    for (const auto i : algo::range(sym_table.size() - 1))
    {
        sym_table[i].occurrences = 1;
        sym_table[i].symbol = i;
    }
    sym_table[sym_table.size() - 1].occurrences = 1;
    sym_table[sym_table.size() - 1].symbol = prob_escape_code;
    for (const auto i : algo::range(sub_model.size()))
    {
        sub_model[i].occurrences = 0;
        sub_model[i].symbol = -1;
    }
}

void ProbModel::increase_symbol(size_t index)
{
    sym_table[index].occurrences++;
    const auto symbol_to_bump = sym_table[index];
    while (index > 0)
    {
        if (sym_table[index - 1].occurrences >= symbol_to_bump.occurrences)
            break;
        sym_table[index] = sym_table[index - 1];
        index--;
    }
    sym_table[index] = symbol_to_bump;
    total_count++;
    if (total_count >= prob_total_limit)
        half_occurrence_count();
}

void ProbModel::half_occurrence_count()
{
    total_count = 0;
    for (const auto i : algo::range(symbol_sorts))
    {
        sym_table[i].occurrences = (sym_table[i].occurrences + 1) >> 1;
        total_count += sym_table[i].occurrences;
    }
    for (const auto i : algo::range(sub_model.size()))
        sub_model[i].occurrences >>= 1;
}

void ProbModel::add_symbol(const s16 symbol)
{
    const auto index = symbol_sorts++;
    sym_table[index].symbol = symbol;
    sym_table[index].occurrences = 1;
    total_count++;
}

s16 ProbModel::find_symbol(const s16 symbol) const
{
    u32 sym = 0;
    while (sym_table[sym].symbol != symbol)
    {
        if (++sym >= symbol_sorts)
            return -1;
    }
    return sym;
}
