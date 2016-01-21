#include "dec/active_soft/custom_bit_stream.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::active_soft;

CustomBitStream::CustomBitStream(const bstr &input) : io::BaseBitStream(input)
{
}

u32 CustomBitStream::read(const size_t bits)
{
    u32 value = 0;
    for (const auto i : algo::range(bits))
    {
        buffer >>= 1;
        if (!bits_available)
        {
            buffer = input_stream->read<u8>();
            bits_available = 8;
        }
        value <<= 1;
        value |= buffer & 1;
        --bits_available;
    }
    position += bits;
    return value;
}
