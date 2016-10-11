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

#include "dec/entis/common/erisan_decoder.h"
#include <array>
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis::common;

static const size_t erisa_prob_slot_max = 0x800;

static const int shift_count[4] = {1, 3, 4, 5};
static const int new_prob_limit[4] = {0x01, 0x08, 0x10, 0x20};

static const size_t nemesis_buf_size = 0x10000;
static const size_t nemesis_index_size = 0x100;

namespace
{
    template<typename T, const size_t n> class CyclicBuffer final
    {
    public:
        CyclicBuffer() : start_pos(0), current_pos(start_pos), a {0} {}
        constexpr size_t size() const { return n; }
        constexpr size_t pos() const { return current_pos; }

        inline void operator <<(const T c)
        {
            a[current_pos++] = c;
            current_pos %= n;
        }

        inline const T &operator [](const size_t i) const
        {
            if (i < n)
                return a[i];
            return a[i % n];
        }

        inline T &operator [](const size_t i)
        {
            if (i < n)
                return a[i];
            return a[i % n];
        }

    private:
        size_t start_pos;
        size_t current_pos;
        std::array<T, n> a;
    };

    using PhraseLookup = CyclicBuffer<size_t, nemesis_index_size>;

    struct ProbBase final
    {
        ProbModel base_model;
        std::array<ProbModel, erisa_prob_slot_max> work;
        size_t work_used;
    };
}

struct ErisaNDecoder::Priv final
{
    CyclicBuffer<u8, 4> last_symbol_buffer;
    ProbBase prob_erisa;

    int nemesis_left;
    int nemesis_next;
    CyclicBuffer<u8, nemesis_buf_size> nemesis_buffer;
    std::array<PhraseLookup, 0x100> nemesis_lookup;
    ProbModel phrase_len_prob;
    ProbModel phrase_index_prob;
    ProbModel run_len_prob;
    bool eof;
};

ErisaNDecoder::ErisaNDecoder() : p(new Priv())
{
}

ErisaNDecoder::~ErisaNDecoder()
{
}

void ErisaNDecoder::reset()
{
    if (!bit_stream)
        throw std::logic_error("Trying to reset with unitialized input");

    code_register = bit_stream->read(32);
    augend_register = 0xFFFF;

    p->nemesis_left = 0;
    p->eof = false;
    for (const auto i : algo::range(4))
        p->last_symbol_buffer << 0;
    for (auto &model : p->prob_erisa.work)
    {
        model.total_count = 0;
        model.symbol_sorts = 0;
        for (auto &sym : model.sym_table)
        {
            sym.symbol = 0;
            sym.occurrences = 0;
        }
        for (auto &sym : model.sub_model)
        {
            sym.symbol = 0;
            sym.occurrences = 0;
        }
    }
    p->prob_erisa.work_used = 0;
}

void ErisaNDecoder::decode(u8 *output, const size_t output_size)
{
    ProbBase *base = &p->prob_erisa;
    if (p->eof)
        return;

    auto output_ptr = algo::make_ptr(output, output_size);
    while (output_ptr.left())
    {
        if (p->nemesis_left > 0)
        {
            const auto nemesis_count = std::min<size_t>(
                output_ptr.end() - output_ptr.current(), p->nemesis_left);
            auto last_symbol = p->nemesis_buffer[p->nemesis_buffer.pos() - 1];

            for (const auto i : algo::range(nemesis_count))
            {
                auto symbol = last_symbol;
                if (p->nemesis_next >= 0)
                    symbol = p->nemesis_buffer[p->nemesis_next++];

                last_symbol = symbol;
                p->nemesis_lookup[symbol] << p->nemesis_buffer.pos();
                p->last_symbol_buffer << symbol;
                p->nemesis_buffer << symbol;
                *output_ptr++ = symbol;
            }

            p->nemesis_left -= nemesis_count;
            continue;
        }

        ProbModel *model = &base->base_model;
        int degree;
        for (degree = 0; degree < 4; degree++)
        {
            const auto last_symbol_index
                = p->last_symbol_buffer[
                    p->last_symbol_buffer.pos() - 1 - degree]
                    >> shift_count[degree];
            if (model->sub_model[last_symbol_index].symbol < 0)
                break;

            model = &base->work.at(model->sub_model[last_symbol_index].symbol);
        }

        auto symbol_index = decode_erisa_code_index(*model);
        if (symbol_index < 0)
            return;

        auto symbol = model->sym_table[symbol_index].symbol;
        model->increase_symbol(symbol_index);

        bool nemesis = false;
        if (symbol == prob_escape_code)
        {
            if (model != &base->base_model)
            {
                symbol_index = decode_erisa_code_index(base->base_model);
                if (symbol_index < 0)
                    return;
                symbol = base->base_model.sym_table[symbol_index].symbol;
                base->base_model.increase_symbol(symbol_index);
                if (symbol != prob_escape_code)
                {
                    model->add_symbol(symbol);
                }
                else
                {
                    nemesis = true;
                }
            }
            else
            {
                nemesis = true;
            }
        }

        if (nemesis)
        {
            const auto phrase_index = decode_erisa_code(p->phrase_index_prob);
            if (phrase_index == prob_escape_code)
            {
                p->eof = true;
                return;
            }
            int size;
            if (phrase_index)
                size = decode_erisa_code(p->phrase_len_prob);
            else
                size = decode_erisa_code(p->run_len_prob);
            if (size == prob_escape_code)
                return;
            const auto last_symbol
                = p->nemesis_buffer[p->nemesis_buffer.pos() - 1];
            const auto &ppl = p->nemesis_lookup[last_symbol];
            p->nemesis_left = size;
            if (phrase_index == 0)
            {
                p->nemesis_next = -1;
            }
            else
            {
                p->nemesis_next = ppl[ppl.pos() - phrase_index];
                if (p->nemesis_buffer[p->nemesis_next] != last_symbol)
                    throw err::CorruptDataError("Expected last symbol");
                p->nemesis_next++;
                p->nemesis_next %= p->nemesis_buffer.size();
            }
            continue;
        }

        p->last_symbol_buffer << symbol;
        p->nemesis_lookup[symbol] << p->nemesis_buffer.pos();
        p->nemesis_buffer << symbol;
        *output_ptr++ = symbol;

        if ((base->work_used >= erisa_prob_slot_max) || (degree >= 4))
            continue;

        symbol_index = symbol >> shift_count[degree];
        model->sub_model.at(symbol_index).occurrences++;
        if (model->sub_model[symbol_index].occurrences < new_prob_limit[degree])
            continue;

        ProbModel *parent = model;
        model = &base->base_model;

        int i;
        for (i = 0; i <= degree; i++)
        {
            symbol_index
                = p->last_symbol_buffer[p->last_symbol_buffer.pos() - 1 - i]
                    >> shift_count[i];
            if (model->sub_model[symbol_index].symbol < 0)
                break;
            model = &base->work.at(model->sub_model[symbol_index].symbol);
        }

        if ((i > degree) || (model->sub_model[symbol_index].symbol >= 0))
            continue;

        auto &new_model = base->work.at(base->work_used);
        model->sub_model[symbol_index].symbol = base->work_used++;
        new_model.total_count = 0;

        int j = 0;
        for (const auto i : algo::range(parent->symbol_sorts))
        {
            const auto occurrences = parent->sym_table[i].occurrences >> 4;
            if (occurrences <= 0)
                continue;
            if (parent->sym_table[i].symbol == prob_escape_code)
                continue;

            new_model.total_count += occurrences;
            new_model.sym_table[j].occurrences = occurrences;
            new_model.sym_table[j].symbol = parent->sym_table[i].symbol;
            j++;
        }

        new_model.total_count++;
        new_model.sym_table[j].occurrences = 1;
        new_model.sym_table[j].symbol = prob_escape_code;
        new_model.symbol_sorts = ++j;
        for (const auto i : algo::range(new_model.sub_model.size()))
        {
            new_model.sub_model[i].occurrences = 0;
            new_model.sub_model[i].symbol = prob_escape_code;
        }
    }
}
