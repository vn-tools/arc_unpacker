#include "dec/entis/common/erisa_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::entis;
using namespace au::dec::entis::common;

struct ErisaDecoder::Priv final
{
    u8 zero_flag;
    size_t available_size;
    ProbModel rle_model;
    ProbModel *last_model;
    std::vector<ProbModel> models;
};

ErisaDecoder::ErisaDecoder() : p(new Priv())
{
}

ErisaDecoder::~ErisaDecoder()
{
}

void ErisaDecoder::reset()
{
    if (!bit_stream)
        throw std::logic_error("Trying to reset with unitialized input");

    code_register = bit_stream->read(32);
    augend_register = 0xFFFF;

    p->models.resize(0x100);
    p->last_model = &p->models[0];
    p->available_size = 0;
}

void ErisaDecoder::decode(u8 *output, const size_t output_size)
{
    if (!bit_stream)
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
