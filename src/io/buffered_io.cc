#include <cassert>
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

char *BufferedIO::buffer()
{
    return p->buffer;
}

void BufferedIO::reserve(size_t length)
{
    size_t new_size = p->buffer_pos + length;
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

void BufferedIO::read(void *destination, size_t length)
{
    if (!length)
        return;
    assert(destination);
    if (p->buffer_pos + length > p->buffer_size)
        throw std::runtime_error("Reading beyond EOF");
    memcpy(destination, p->buffer + p->buffer_pos, length);
    p->buffer_pos += length;
}

void BufferedIO::write(const void *source, size_t length)
{
    if (!length)
        return;
    assert(source);
    reserve(length);
    memcpy(p->buffer + p->buffer_pos, source, length);
    p->buffer_pos += length;
}

void BufferedIO::write_from_io(IO &source, size_t length)
{
    reserve(length);
    source.read(p->buffer + p->buffer_pos, length);
    p->buffer_pos += length;
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

BufferedIO::BufferedIO(IO &other_io, size_t length)
    : p(new Priv("", 0))
{
    write_from_io(other_io, length);
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
