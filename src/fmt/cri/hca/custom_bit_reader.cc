#include "fmt/cri/hca/custom_bit_reader.h"

using namespace au;
using namespace au::fmt::cri::hca;

static const int mask[] =
{
    0xFFFFFF,
    0x7FFFFF,
    0x3FFFFF,
    0x1FFFFF,
    0x0FFFFF,
    0x07FFFF,
    0x03FFFF,
    0x01FFFF
};

struct CustomBitReader::Priv final
{
    Priv(const bstr &data);

    bstr data;
    size_t size;
    size_t pos;
};

CustomBitReader::Priv::Priv(const bstr &data)
    : data(data), size(data.size() * 8), pos(0)
{
}

CustomBitReader::CustomBitReader(const bstr &data) : p(new Priv(data))
{
}

CustomBitReader::~CustomBitReader()
{
}

int CustomBitReader::get(const size_t n)
{
    int ret = 0;
    if (p->pos + n <= p->size)
    {
        u8 *data = &p->data[p->pos >> 3];
        ret = data[0];
        ret = (ret << 8) | data[1];
        ret = (ret << 8) | data[2];
        ret &= mask[p->pos & 7];
        ret >>= 24 - (p->pos & 7) - n;
    }
    p->pos += n;
    return ret;
}

void CustomBitReader::skip(const int n)
{
	p->pos += n;
}
