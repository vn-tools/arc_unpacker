#include "dec/entis/common/base_erisa_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis::common;

int BaseErisaDecoder::decode_erisa_code(ProbModel &model)
{
    const auto index = decode_erisa_code_index(model);
    if (index < 0)
        return prob_escape_code;
    const auto symbol = model.sym_table[index].symbol;
    model.increase_symbol(index);
    return symbol;
}

int BaseErisaDecoder::decode_erisa_code_index(const ProbModel &model)
{
    if (!bit_reader)
    {
        throw std::logic_error(
            "Trying to decode ERISA code index with unitialized input");
    }

    u32 acc = code_register * model.total_count / augend_register;
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
    code_register -= (augend_register * fs + model.total_count - 1)
        / model.total_count;
    augend_register = augend_register * occurences / model.total_count;
    if (augend_register == 0)
        throw err::CorruptDataError("Empty augend register");

    while (!(augend_register & 0x8000))
    {
        code_register <<= 1;
        code_register |= bit_reader->get(1);
        augend_register <<= 1;
    }

    code_register &= 0xFFFF;
    return symbol_index;
}
