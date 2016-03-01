#include "io/memory_stream.h"
#include <cstring>
#include "err.h"

using namespace au;
using namespace au::io;

MemoryStream::MemoryStream(const std::shared_ptr<bstr> input)
    : buffer(input), buffer_pos(0)
{
}

MemoryStream::MemoryStream() : MemoryStream(std::make_shared<bstr>(""_b))
{
}

MemoryStream::MemoryStream(const bstr &buffer)
    : MemoryStream(std::make_shared<bstr>(buffer))
{
}

MemoryStream::MemoryStream(const char *buffer, const size_t buffer_size)
    : MemoryStream(std::make_shared<bstr>(buffer, buffer_size))
{
}

MemoryStream::MemoryStream(io::BaseByteStream &other, const size_t size)
    : MemoryStream(std::make_shared<bstr>(other.read(size)))
{
}

MemoryStream::MemoryStream(io::BaseByteStream &other)
    : MemoryStream(std::make_shared<bstr>(other.read_to_eof()))
{
}

MemoryStream::~MemoryStream()
{
}

io::BaseByteStream &MemoryStream::reserve(const uoff_t size)
{
    if (buffer->size() < size)
        buffer->resize(size);
    return *this;
}

void MemoryStream::seek_impl(const uoff_t offset)
{
    if (offset > buffer->size())
        throw err::EofError();
    buffer_pos = offset;
}

void MemoryStream::read_impl(void *destination, const size_t size)
{
    // destination MUST exist and size MUST be at least 1
    if (buffer_pos + size > buffer->size())
        throw err::EofError();
    auto source_ptr = buffer->get<const u8>() + buffer_pos;
    auto destination_ptr = reinterpret_cast<u8*>(destination);
    buffer_pos += size;
    std::memcpy(destination_ptr, source_ptr, size);
}

void MemoryStream::write_impl(const void *source, size_t size)
{
    // source MUST exist and size MUST be at least 1
    reserve(buffer_pos + size);
    auto source_ptr = reinterpret_cast<const u8*>(source);
    auto destination_ptr = buffer->get<u8>() + buffer_pos;
    buffer_pos += size;
    std::memcpy(destination_ptr, source_ptr, size);
}

uoff_t MemoryStream::pos() const
{
    return buffer_pos;
}

uoff_t MemoryStream::size() const
{
    return buffer->size();
}

void MemoryStream::resize_impl(const uoff_t new_size)
{
    buffer->resize(new_size);
    if (buffer_pos > new_size)
        buffer_pos = new_size;
}

std::unique_ptr<io::BaseByteStream> MemoryStream::clone() const
{
    auto ret = std::unique_ptr<MemoryStream>(new MemoryStream(buffer));
    ret->seek(pos());
    return std::move(ret);
}
