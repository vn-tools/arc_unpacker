#include "fmt/entis/common/prob_model.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::entis::common;

ProbModel::ProbModel()
{
    total_count = prob_symbol_sorts;
    symbol_sorts = prob_symbol_sorts;
    for (auto i : util::range(prob_symbol_sorts - 1))
    {
        sym_table[i].occurrences = 1;
        sym_table[i].symbol = i;
    }
    sym_table[prob_symbol_sorts - 1].occurrences = 1;
    sym_table[prob_symbol_sorts - 1].symbol = prob_escape_code;
}

void ProbModel::increase_symbol(int index)
{
    ++sym_table[index].occurrences;
    auto symbol_to_bump = sym_table[index];
    while (index > 0)
    {
        if (sym_table[index - 1].occurrences >= symbol_to_bump.occurrences)
            break;
        sym_table[index] = sym_table[index - 1];
        --index;
    }
    sym_table[index] = symbol_to_bump;
    total_count++;
    if (total_count >= prob_total_limit)
        half_occurrence_count();
}

void ProbModel::half_occurrence_count()
{
    total_count = 0;
    for (auto i : util::range(symbol_sorts))
    {
        sym_table[i].occurrences = (sym_table[i].occurrences + 1) >> 1;
        total_count += sym_table[i].occurrences;
    }
}

i16 ProbModel::find_symbol(i16 symbol)
{
    u32 sym = 0;
    while (sym_table[sym].symbol != symbol)
    {
        if (++sym >= symbol_sorts)
            return -1;
    }
    return sym;
}
