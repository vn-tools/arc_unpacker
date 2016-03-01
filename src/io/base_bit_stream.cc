#include "io/base_bit_stream.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::io;

BaseBitStream::~BaseBitStream() {}

BaseBitStream::BaseBitStream(const bstr &input) :
    buffer(0),
    bits_available(0),
    position(0),
    own_stream_holder(new MemoryStream(input)),
    input_stream(own_stream_holder.get())
{
}

BaseBitStream::BaseBitStream(io::BaseByteStream &input_stream) :
    buffer(0),
    bits_available(0),
    position(0),
    input_stream(&input_stream)
{
}

BaseStream &BaseBitStream::seek(const uoff_t new_pos)
{
    if (new_pos > size())
        throw err::EofError();
    position = (new_pos / 32) * 32;
    bits_available = 0;
    buffer = 0;
    input_stream->seek(position / 8);
    read(new_pos % 32);
    return *this;
}

BaseStream &BaseBitStream::resize(const uoff_t new_size)
{
    throw err::NotSupportedError("Not implemented");
}

uoff_t BaseBitStream::pos() const
{
    return position;
}

uoff_t BaseBitStream::size() const
{
    return input_stream->size() * 8;
}

// Elias Gamma coding
u32 BaseBitStream::read_gamma(const bool stop_mark)
{
    size_t count = 0;
    while (read(1) != stop_mark)
        ++count;
    u32 value = 1;
    while (count--)
    {
        value <<= 1;
        value |= read(1); // one by one to enforce MSB order
    }
    return value;
}

void BaseBitStream::flush()
{
}

void BaseBitStream::write(const size_t bits, const u32 value)
{
    throw err::NotSupportedError("Not implemented");
}
