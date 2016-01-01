#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::io;

MsbBitReader::MsbBitReader(const bstr &input) : BaseBitReader(input)
{
}

MsbBitReader::MsbBitReader(io::IStream &input_stream)
    : BaseBitReader(input_stream)
{
}

u32 MsbBitReader::get(const size_t bits)
{
    while (bits_available < bits)
    {
        const auto tmp = input_stream->read_u8();
        buffer = (buffer << 8) | tmp;
        bits_available += 8;
    }
    const auto mask = (1ull << bits) - 1;
    bits_available -= bits;
    position += bits;
    return (buffer >> bits_available) & mask;
}
