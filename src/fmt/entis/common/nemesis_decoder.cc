#include "fmt/entis/common/nemesis_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::fmt::entis;
using namespace au::fmt::entis::common;

struct NemesisDecoder::Priv final
{
    u8 zero_flag;
    size_t available_size;

    u32 code_register;
    u32 augend_register;
    ProbModel rle_model;
    ProbModel *last_model;
    std::vector<ProbModel> models;
};

NemesisDecoder::NemesisDecoder() : p(new Priv())
{
}

NemesisDecoder::~NemesisDecoder()
{
}

void NemesisDecoder::reset()
{
    if (!bit_reader)
        throw std::logic_error("Trying to reset with unitialized input");
    p->models.resize(0x100);
	p->last_model = &p->models[0];
    p->available_size = 0;
    p->code_register = bit_reader->get(32);
    p->augend_register = 0xFFFF;
}

void NemesisDecoder::decode(u8 *output, size_t output_size)
{
    if (!bit_reader)
        throw std::logic_error("Trying to reset with unitialized input");

    auto output_ptr = output;
    auto output_end = output + output_size;
    auto current_model = p->last_model;
    while (output_ptr < output_end)
    {
        if (p->available_size)
        {
            auto size = std::min<size_t>(
                output_end - output_ptr, p->available_size);
            p->available_size -= size;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = 0;
            continue;
        }

        auto symbol_index = decode_erisa_code_index(*current_model);
        if (symbol_index < 0)
            break;
        auto symbol = current_model->sym_table[symbol_index].symbol;
        current_model->increase_symbol(symbol_index);
        *output_ptr++ = symbol;
        if (!symbol)
        {
            symbol_index = decode_erisa_code_index(p->rle_model);
            if (symbol_index < 0)
                break;
            p->available_size = p->rle_model.sym_table[symbol_index].symbol;
            p->rle_model.increase_symbol(symbol_index);
        }
        current_model = &p->models[symbol & 0xFF];
    }
    p->last_model = current_model;
}

int NemesisDecoder::decode_erisa_code(ProbModel &model)
{
    int index = decode_erisa_code_index(model);
    int symbol = prob_escape_code;
    if (index >= 0)
    {
        symbol = model.sym_table[index].symbol;
        model.increase_symbol(index);
    }
    return symbol;
}

int NemesisDecoder::decode_erisa_code_index(const ProbModel &model)
{
    if (!bit_reader)
        throw std::logic_error("Trying to reset with unitialized input");

    u32 acc = p->code_register * model.total_count / p->augend_register;
    if (acc >= prob_total_limit)
        return prob_escape_code;

    size_t symbol_index = 0;
    u16 fs = 0;
    u16 occurences;
    while (true)
    {
        occurences = model.sym_table[symbol_index].occurrences;
        if (acc < occurences)
            break;
        acc -= occurences;
        fs += occurences;
        if (++symbol_index >= model.symbol_sorts)
            return prob_escape_code;
    }
    p->code_register -= (p->augend_register * fs + model.total_count - 1)
        / model.total_count;
    p->augend_register = p->augend_register * occurences / model.total_count;

    while (!(p->augend_register & 0x8000))
    {
        p->code_register <<= 1;
        p->code_register |= bit_reader->get(1);
        p->augend_register <<= 1;
    }

    p->code_register &= 0xFFFF;
    return symbol_index;
}
