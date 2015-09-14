#include "err.h"
#include "io/buffered_io.h"

using namespace au::io;

struct BufferedIO::Priv final
{
    bstr buffer;
    size_t buffer_pos;

    Priv(bstr buffer) : buffer(buffer), buffer_pos(0)
    {
    }
};

BufferedIO::BufferedIO() : p(new Priv(""_b))
{
}

BufferedIO::BufferedIO(const bstr &buffer) : p(new Priv(buffer))
{
}

BufferedIO::BufferedIO(const char *buffer, size_t buffer_size)
    : p(new Priv(bstr(buffer, buffer_size)))
{
}

BufferedIO::BufferedIO(IO &other_io, size_t size) : p(new Priv(""_b))
{
    write_from_io(other_io, size);
    seek(0);
}

BufferedIO::BufferedIO(IO &other_io) : p(new Priv(""_b))
{
    write_from_io(other_io, other_io.size() - other_io.tell());
    seek(0);
}

BufferedIO::~BufferedIO()
{
}

void BufferedIO::reserve(size_t size)
{
    if (p->buffer.size() < size)
        p->buffer.resize(size);
}

void BufferedIO::seek(size_t offset)
{
    if (offset > p->buffer.size())
        throw err::IoError("Seeking beyond EOF");
    p->buffer_pos = offset;
}

void BufferedIO::skip(int offset)
{
    if (p->buffer_pos + offset > p->buffer.size())
        throw err::IoError("Seeking beyond EOF");
    p->buffer_pos += offset;
}

void BufferedIO::read(void *destination, size_t size)
{
    if (!size)
        return;
    if (!destination)
        throw std::logic_error("Reading to nullptr");
    if (p->buffer_pos + size > p->buffer.size())
        throw err::IoError("Reading beyond EOF");
    auto source_ptr = p->buffer.get<u8>() + p->buffer_pos;
    auto destination_ptr = reinterpret_cast<u8*>(destination);
    p->buffer_pos += size;
    while (size--)
        *destination_ptr++ = *source_ptr++;
}

void BufferedIO::write(const void *source, size_t size)
{
    if (!size)
        return;
    if (!source)
        throw std::logic_error("Writing from nullptr");
    reserve(p->buffer_pos + size);
    auto source_ptr = reinterpret_cast<const u8*>(source);
    auto destination_ptr = p->buffer.get<u8>() + p->buffer_pos;
    p->buffer_pos += size;
    while (size--)
        *destination_ptr++ = *source_ptr++;
}

void BufferedIO::write_from_io(IO &source, size_t size)
{
    reserve(p->buffer_pos + size);
    source.read(p->buffer.get<u8>() + p->buffer_pos, size);
    p->buffer_pos += size;
}

size_t BufferedIO::tell() const
{
    return p->buffer_pos;
}

size_t BufferedIO::size() const
{
    return p->buffer.size();
}

void BufferedIO::truncate(size_t new_size)
{
    p->buffer.resize(new_size);
    if (p->buffer_pos > new_size)
        p->buffer_pos = new_size;
}
