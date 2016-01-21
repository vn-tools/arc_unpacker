#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::io;

MsbBitStream::MsbBitStream(const bstr &input) : BaseBitStream(input)
{
}

MsbBitStream::MsbBitStream(io::BaseByteStream &input_stream)
    : BaseBitStream(input_stream)
{
}

u32 MsbBitStream::read(const size_t bits)
{
    while (bits_available < bits)
    {
        const auto tmp = input_stream->read<u8>();
        buffer = (buffer << 8) | tmp;
        bits_available += 8;
    }
    const auto mask = (1ull << bits) - 1;
    bits_available -= bits;
    position += bits;
    return (buffer >> bits_available) & mask;
}
