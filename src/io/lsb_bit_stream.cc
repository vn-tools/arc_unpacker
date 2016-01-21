#include "io/lsb_bit_stream.h"

using namespace au;
using namespace au::io;

LsbBitStream::LsbBitStream(const bstr &input) : BaseBitStream(input)
{
}

LsbBitStream::LsbBitStream(io::BaseByteStream &input_stream)
    : BaseBitStream(input_stream)
{
}

u32 LsbBitStream::read(const size_t bits)
{
    while (bits_available < bits)
    {
        const auto tmp = input_stream->read<u8>();
        buffer |= tmp << bits_available;
        bits_available += 8;
    }
    const auto mask = (1ull << bits) - 1;
    const auto value = buffer & mask;
    buffer >>= bits;
    bits_available -= bits;
    position += bits;
    return value;
}
