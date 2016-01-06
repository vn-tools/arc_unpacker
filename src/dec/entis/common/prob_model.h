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
