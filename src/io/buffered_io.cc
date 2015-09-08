#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "io/buffered_io.h"

using namespace au::io;

struct BufferedIO::Priv
{
    char *buffer;
    size_t buffer_size;
    size_t buffer_pos;

    Priv(const char *buffer, size_t buffer_size)
    {
        this->buffer = reinterpret_cast<char*>(malloc(buffer_size));
        if (!this->buffer)
            throw std::bad_alloc();
        memcpy(this->buffer, buffer, buffer_size);
        this->buffer_pos = 0;
        this->buffer_size = buffer_size;
    }
};

void BufferedIO::reserve(size_t size)
{
    size_t new_size = p->buffer_pos + size;
    if (new_size <= p->buffer_size)
        return;
    char *new_buffer = reinterpret_cast<char*>(
        realloc(p->buffer, new_size));
    if (!new_buffer)
        throw std::bad_alloc();
    p->buffer = new_buffer;
    p->buffer_size = new_size;
}

void BufferedIO::seek(size_t offset)
{
    if (offset > p->buffer_size)
        throw std::runtime_error("Seeking beyond EOF");
    p->buffer_pos = offset;
}

void BufferedIO::skip(int offset)
{
    if (p->buffer_pos + offset > p->buffer_size)
        throw std::runtime_error("Seeking beyond EOF");
    p->buffer_pos += offset;
}

void BufferedIO::read(void *destination, size_t size)
{
    if (!size)
        return;
    if (!destination)
        throw std::logic_error("Reading to nullptr");
    if (p->buffer_pos + size > p->buffer_size)
        throw std::runtime_error("Reading beyond EOF");
    memcpy(destination, p->buffer + p->buffer_pos, size);
    p->buffer_pos += size;
}

void BufferedIO::write(const void *source, size_t size)
{
    if (!size)
        return;
    if (!source)
        throw std::logic_error("Writing from nullptr");
    reserve(size);
    memcpy(p->buffer + p->buffer_pos, source, size);
    p->buffer_pos += size;
}

void BufferedIO::write_from_io(IO &source, size_t size)
{
    reserve(size);
    source.read(p->buffer + p->buffer_pos, size);
    p->buffer_pos += size;
}

size_t BufferedIO::tell() const
{
    return p->buffer_pos;
}

size_t BufferedIO::size() const
{
    return p->buffer_size;
}

void BufferedIO::truncate(size_t new_size)
{
    if (new_size == 0)
    {
        free(p->buffer);
        p->buffer_size = 0;
        p->buffer_pos = 0;
        p->buffer = nullptr;
    }
    char *new_buffer = reinterpret_cast<char*>(
        realloc(p->buffer, new_size));
    if (!new_buffer)
        throw std::bad_alloc();
    p->buffer = new_buffer;
    p->buffer_size = new_size;
    if (p->buffer_pos > new_size)
        p->buffer_pos = new_size;
}

BufferedIO::BufferedIO() : p(new Priv("", 0))
{
}

BufferedIO::BufferedIO(IO &other_io, size_t size)
    : p(new Priv("", 0))
{
    write_from_io(other_io, size);
    seek(0);
}

BufferedIO::BufferedIO(IO &other_io)
    : p(new Priv("", 0))
{
    write_from_io(other_io, other_io.size() - other_io.tell());
    seek(0);
}

BufferedIO::BufferedIO(const bstr &buffer)
    : p(new Priv(buffer.get<char>(), buffer.size()))
{
}

BufferedIO::BufferedIO(const char *buffer, size_t buffer_size)
    : p(new Priv(buffer, buffer_size))
{
}

BufferedIO::~BufferedIO()
{
    if (p)
        free(p->buffer);
}
