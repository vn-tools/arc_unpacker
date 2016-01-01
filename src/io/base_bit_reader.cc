#include "io/base_bit_reader.h"
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

BaseBitReader::BaseBitReader(IStream &input_stream) :
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

// Elias Gamma coding
u32 BaseBitReader::get_gamma(const bool stop_mark)
{
    size_t count = 0;
    while (get(1) != stop_mark)
        ++count;
    u32 value = 1;
    while (count--)
    {
        value <<= 1;
        value |= get(1); // one by one to enforce MSB order
    }
    return value;
}
