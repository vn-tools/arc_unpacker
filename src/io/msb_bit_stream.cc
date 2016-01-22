#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::io;

MsbBitStream::MsbBitStream(const bstr &input)
    : BaseBitStream(input), dirty(false)
{
}

MsbBitStream::MsbBitStream(io::BaseByteStream &input_stream)
    : BaseBitStream(input_stream), dirty(false)
{
}

MsbBitStream::~MsbBitStream()
{
    if (dirty)
        input_stream->write<u8>(buffer << (8 - bits_available));
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

void MsbBitStream::write(const size_t bits, const u32 value)
{
    const auto mask = (1ull << bits) - 1;
    buffer <<= bits;
    buffer |= value & mask;
    bits_available += bits;
    position += bits;
    while (bits_available > 8)
    {
        bits_available -= 8;
        input_stream->write<u8>(buffer >> bits_available);
    }
    dirty = bits_available > 0;
}
