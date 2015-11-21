#include "io/memory_stream.h"
#include <cstring>
#include "err.h"

using namespace au;
using namespace au::io;

struct MemoryStream::Priv final
{
    Priv(bstr buffer);

    template<typename T> inline T read_primitive()
    {
        const auto size = sizeof(T);
        if (buffer_pos + size > buffer.size())
            throw err::EofError();
        const auto ret = reinterpret_cast<const u32&>(buffer[buffer_pos]);
        buffer_pos += size;
        return ret;
    }

    bstr buffer;
    size_t buffer_pos;
};

MemoryStream::Priv::Priv(bstr buffer) : buffer(buffer), buffer_pos(0)
{
}

MemoryStream::MemoryStream() : p(new Priv(""_b))
{
}

MemoryStream::MemoryStream(const bstr &buffer) : p(new Priv(buffer))
{
}

MemoryStream::MemoryStream(const char *buffer, size_t buffer_size)
    : p(new Priv(bstr(buffer, buffer_size)))
{
}

MemoryStream::MemoryStream(Stream &other, size_t size)
    : p(new Priv(other.read(size)))
{
}

MemoryStream::MemoryStream(Stream &other)
    : p(new Priv(other.read_to_eof()))
{
}

MemoryStream::~MemoryStream()
{
}

void MemoryStream::reserve(size_t size)
{
    if (p->buffer.size() < size)
        p->buffer.resize(size);
}

Stream &MemoryStream::seek(size_t offset)
{
    if (offset > p->buffer.size())
        throw err::EofError();
    p->buffer_pos = offset;
    return *this;
}

Stream &MemoryStream::skip(int offset)
{
    if (p->buffer_pos + offset > p->buffer.size())
        throw err::EofError();
    p->buffer_pos += offset;
    return *this;
}

u8 MemoryStream::read_u8()
{
    return p->read_primitive<u8>();
}

u16 MemoryStream::read_u16_le()
{
    return p->read_primitive<u16>();
}

u32 MemoryStream::read_u32_le()
{
    return p->read_primitive<u32>();
}

void MemoryStream::read_impl(void *destination, size_t size)
{
    // destination MUST exist and size MUST be at least 1
    if (p->buffer_pos + size > p->buffer.size())
        throw err::EofError();
    auto source_ptr = p->buffer.get<u8>() + p->buffer_pos;
    auto destination_ptr = reinterpret_cast<u8*>(destination);
    p->buffer_pos += size;
    std::memcpy(destination_ptr, source_ptr, size);
}

void MemoryStream::write_impl(const void *source, size_t size)
{
    // source MUST exist and size MUST be at least 1
    reserve(p->buffer_pos + size);
    auto source_ptr = reinterpret_cast<const u8*>(source);
    auto destination_ptr = p->buffer.get<u8>() + p->buffer_pos;
    p->buffer_pos += size;
    while (size--)
        *destination_ptr++ = *source_ptr++;
}

size_t MemoryStream::tell() const
{
    return p->buffer_pos;
}

size_t MemoryStream::size() const
{
    return p->buffer.size();
}

void MemoryStream::truncate(size_t new_size)
{
    p->buffer.resize(new_size);
    if (p->buffer_pos > new_size)
        p->buffer_pos = new_size;
}
