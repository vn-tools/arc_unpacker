#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "io/buffered_io.h"

struct BufferedIO::Internals
{
    char *buffer;
    size_t buffer_size;
    size_t buffer_pos;

    Internals(const char *buffer, size_t buffer_size)
    {
        this->buffer = reinterpret_cast<char*>(malloc(buffer_size));
        if (this->buffer == nullptr)
            throw std::bad_alloc();
        memcpy(this->buffer, buffer, buffer_size);
        this->buffer_pos = 0;
        this->buffer_size = buffer_size;
    }
};

char *BufferedIO::buffer()
{
    return internals->buffer;
}

void BufferedIO::reserve(size_t length)
{
    size_t new_size = internals->buffer_pos + length;
    if (new_size <= internals->buffer_size)
        return;
    char *new_buffer = reinterpret_cast<char*>(
        realloc(internals->buffer, new_size));
    if (new_buffer == nullptr)
        throw std::bad_alloc();
    internals->buffer = new_buffer;
    internals->buffer_size = new_size;
}

void BufferedIO::seek(size_t offset)
{
    if (offset > internals->buffer_size)
        throw std::runtime_error("Seeking beyond EOF");
    internals->buffer_pos = offset;
}

void BufferedIO::skip(int offset)
{
    if (internals->buffer_pos + offset > internals->buffer_size)
        throw std::runtime_error("Seeking beyond EOF");
    internals->buffer_pos += offset;
}

void BufferedIO::read(void *destination, size_t length)
{
    assert(destination != nullptr);
    if (internals->buffer_pos + length > internals->buffer_size)
        throw std::runtime_error("Reading beyond EOF");
    memcpy(destination, internals->buffer + internals->buffer_pos, length);
    internals->buffer_pos += length;
}

void BufferedIO::write(const void *source, size_t length)
{
    assert(source != nullptr);
    reserve(length);
    memcpy(internals->buffer + internals->buffer_pos, source, length);
    internals->buffer_pos += length;
}

void BufferedIO::write_from_io(IO &source, size_t length)
{
    reserve(length);
    source.read(internals->buffer + internals->buffer_pos, length);
    internals->buffer_pos += length;
}

size_t BufferedIO::tell() const
{
    return internals->buffer_pos;
}

size_t BufferedIO::size() const
{
    return internals->buffer_size;
}

void BufferedIO::truncate(size_t new_size)
{
    if (new_size == 0)
    {
        free(internals->buffer);
        internals->buffer_size = 0;
        internals->buffer_pos = 0;
        internals->buffer = nullptr;
    }
    char *new_buffer = reinterpret_cast<char*>(
        realloc(internals->buffer, new_size));
    if (new_buffer == nullptr)
        throw std::bad_alloc();
    internals->buffer = new_buffer;
    internals->buffer_size = new_size;
    if (internals->buffer_pos > new_size)
        internals->buffer_pos = new_size;
}

BufferedIO::BufferedIO() : internals(new Internals("", 0))
{
}

BufferedIO::BufferedIO(IO &other_io, size_t length)
    : internals(new Internals("", 0))
{
    write_from_io(other_io, length);
    seek(0);
}

BufferedIO::BufferedIO(IO &other_io)
    : internals(new Internals("", 0))
{
    write_from_io(other_io, other_io.size() - other_io.tell());
    seek(0);
}

BufferedIO::BufferedIO(const std::string &buffer)
    : internals(new Internals(buffer.data(), buffer.size()))
{
}

BufferedIO::BufferedIO(const char *buffer, size_t buffer_size)
    : internals(new Internals(buffer, buffer_size))
{
}

BufferedIO::~BufferedIO()
{
    if (internals != nullptr)
        free(internals->buffer);
}
