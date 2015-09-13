#include "err.h"
#include "fmt/entis/common/gamma_decoder.h"

using namespace au;
using namespace au::fmt::entis;
using namespace au::fmt::entis::common;

int common::get_gamma_code(io::BitReader &bit_reader)
{
    if (bit_reader.eof())
        return 0;
    if (!bit_reader.get(1))
        return 1;
    auto base = 2;
    auto code = 0;
    while (true)
    {
        if (bit_reader.eof())
            return 0;
        code = (code << 1) | bit_reader.get(1);
        if (bit_reader.eof())
            return 0;
        if (!bit_reader.get(1))
            return code + base;
        base <<= 1;
    }
}

struct GammaDecoder::Priv
{
    Priv(io::BitReader &bit_reader);
    ~Priv();

    void init();
    void decode(u8 *output, size_t output_size);

    io::BitReader &bit_reader;
    u8 zero_flag;
    size_t available_size;
};

GammaDecoder::Priv::Priv(io::BitReader &bit_reader) : bit_reader(bit_reader)
{
    zero_flag = 0;
}

GammaDecoder::Priv::~Priv()
{
}

void GammaDecoder::Priv::init()
{
    zero_flag = bit_reader.get(1);
    available_size = 0;
}

void GammaDecoder::Priv::decode(u8 *output, size_t output_size)
{
    auto output_ptr = output;
    auto output_end = output + output_size;

    if (!available_size)
    {
        available_size = get_gamma_code(bit_reader);
        if (!available_size)
            return;
    }

    while (output_ptr < output_end)
    {
        unsigned int size = std::min(available_size, output_size);
        available_size -= size;
        output_size -= size;

        if (!zero_flag)
        {
            while (size--)
                *output_ptr++ = 0;
        }
        else
        {
            while (size--)
            {
                auto sign = bit_reader.get(1) ? 0xFF : 0;
                auto code = get_gamma_code(bit_reader);
                if (!code)
                    return;
                if (output_ptr >= output_end)
                    throw err::BadDataOffsetError();
                u8 out = (code ^ sign) - sign;

                //std::cerr << util::format("bit:%d,",sign);
                //std::cerr << util::format("code:%02x\n",code);
                //std::cerr << util::format("out:%02x\n",out);
                *output_ptr++ = out;
            }
        }
        if (!output_size)
        {
            if (!available_size)
                zero_flag ^= 1;
            return;
        }
        zero_flag ^= 1;
        available_size = get_gamma_code(bit_reader);
        if (!available_size)
            return;
    }
}

GammaDecoder::GammaDecoder(const bstr &data)
    : Decoder(data), p(new Priv(bit_reader))
{
}

GammaDecoder::~GammaDecoder()
{
}

void GammaDecoder::init()
{
    p->init();
}

void GammaDecoder::decode(u8 *output, size_t output_size)
{
    p->decode(output, output_size);
}
