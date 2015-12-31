#include "fmt/active_soft/custom_bit_reader.h"
#include "algo/range.h"

using namespace au;
using namespace au::fmt::active_soft;

CustomBitReader::CustomBitReader(const bstr &input) : io::BaseBitReader(input)
{
}

u32 CustomBitReader::get(const size_t bits)
{
    u32 value = 0;
    for (const auto i : algo::range(bits))
    {
        buffer >>= 1;
        if (!bits_available)
        {
            buffer = input_stream->read_u8();
            bits_available = 8;
        }
        value <<= 1;
        value |= buffer & 1;
        --bits_available;
    }
    position += bits;
    return value;
}
