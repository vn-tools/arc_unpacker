#include "fmt/active_soft/custom_bit_reader.h"
#include "io/buffered_io.h"

using namespace au;
using namespace au::fmt::active_soft;

struct CustomBitReader::Priv final
{
    Priv(const bstr &input);
    io::BufferedIO input_io;
    u32 value;
};

CustomBitReader::Priv::Priv(const bstr &input) : input_io(input), value(0)
{
}

CustomBitReader::CustomBitReader(const bstr &input) : p(new Priv(input))
{
}

CustomBitReader::~CustomBitReader()
{
}

u32 CustomBitReader::get(size_t bits)
{
    u32 ret = 0;
    u32 value = p->value;
    while (bits--)
    {
        value >>= 1;
        if (!(value & 0xF00))
            value = p->input_io.read_u8() | 0xFF00;
        ret <<= 1;
        ret |= value & 1;
    }
    p->value = value;
    return ret;
}

u32 CustomBitReader::get_variable_integer()
{
    int bit = 1, count = 0;
    while (count < 32 && bit)
    {
        bit = get(1);
        count++;
    }
    --count;
    if (count)
        return (1 << count) | get(count);
    return 1;
}
