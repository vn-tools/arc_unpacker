#include "util/cyclic_buffer.h"

using namespace au;
using namespace au::util;

struct CyclicBuffer::Priv final
{
    Priv(const size_t size, const size_t start_pos);
    bstr buffer;
    u8 *buffer_ptr;
    const u8 *buffer_start; // points to the beginning of the internal storage
    const u8 *buffer_end;
    size_t start_pos;
    int written;
    size_t size;
};

CyclicBuffer::Priv::Priv(const size_t size, const size_t start_pos)
    : buffer(size),
    buffer_ptr(buffer.get<u8>() + (start_pos % size)),
    buffer_start(buffer.get<const u8>()),
    buffer_end(buffer.end<const u8>()),
    start_pos(start_pos % size),
    written((start_pos % size) - size),
    size(size)
{
}

CyclicBuffer::CyclicBuffer(const size_t size, const size_t start_pos)
    : p(new Priv(size, start_pos))
{
}

CyclicBuffer::~CyclicBuffer()
{
}

size_t CyclicBuffer::size() const
{
    return p->buffer.size();
}

size_t CyclicBuffer::start() const
{
    return p->written < 0 ? p->start_pos : (p->written % p->buffer.size());
}

size_t CyclicBuffer::pos() const
{
    return p->buffer_ptr - p->buffer_start;
}

void CyclicBuffer::operator <<(const bstr &s)
{
    auto repetitions = s.size();
    auto input_ptr = s.get<const u8>();
    const auto buffer_end = p->buffer_end;
    auto buffer_ptr = p->buffer_ptr;
    p->written += repetitions;
    while (repetitions--)
    {
        *buffer_ptr++ = *input_ptr++;
        if (buffer_ptr == buffer_end)
            buffer_ptr = p->buffer.get<u8>();
    }
    p->buffer_ptr = buffer_ptr;
}

void CyclicBuffer::operator <<(const u8 c)
{
    *p->buffer_ptr++ = c;
    if (p->buffer_ptr == p->buffer_end)
        p->buffer_ptr = p->buffer.get<u8>();
    p->written += 1;
}

u8 &CyclicBuffer::operator [](const size_t n)
{
    u8 &tmp = n >= p->size ? p->buffer[n % p->size] : p->buffer[n];
    return tmp;
}

const u8 &CyclicBuffer::operator [](const size_t n) const
{
    const u8 &tmp = p->buffer[n];
    return tmp;
}
