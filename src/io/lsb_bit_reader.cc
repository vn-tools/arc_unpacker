#include "io/lsb_bit_reader.h"

using namespace au;
using namespace au::io;

LsbBitReader::LsbBitReader(const bstr &input) : BaseBitReader(input)
{
}

LsbBitReader::LsbBitReader(io::IStream &input_stream)
    : BaseBitReader(input_stream)
{
}

u32 LsbBitReader::get(const size_t bits)
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
