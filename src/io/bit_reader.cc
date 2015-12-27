#include "io/bit_reader.h"
#include <algorithm>
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::io;

BaseBitReader::BaseBitReader(const bstr &input) :
    buffer(0),
    bits_available(0),
    position(0),
    own_stream_holder(new MemoryStream(input)),
    input_stream(own_stream_holder.get())
{
}

BaseBitReader::BaseBitReader(Stream &input_stream) :
    buffer(0),
    bits_available(0),
    position(0),
    input_stream(&input_stream)
{
}

void BaseBitReader::seek(const size_t new_pos)
{
    if (new_pos > size())
        throw err::EofError();
    position = (new_pos / 32) * 32;
    bits_available = 0;
    buffer = 0;
    input_stream->seek(position / 8);
    get(new_pos % 32);
}

void BaseBitReader::skip(int offset)
{
    return seek(tell() + offset);
}

size_t BaseBitReader::tell() const
{
    return position;
}

bool BaseBitReader::eof() const
{
    return position == size();
}

size_t BaseBitReader::size() const
{
    return input_stream->size() * 8;
}

LsbBitReader::LsbBitReader(const bstr &input) : BaseBitReader(input)
{
}

LsbBitReader::LsbBitReader(io::Stream &input_stream)
    : BaseBitReader(input_stream)
{
}

u32 LsbBitReader::get(const size_t bits)
{
    while (bits_available < bits)
    {
        const auto tmp = input_stream->read_u8();
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

MsbBitReader::MsbBitReader(const bstr &input) : BaseBitReader(input)
{
}

MsbBitReader::MsbBitReader(io::Stream &input_stream)
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
