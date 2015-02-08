#include "bit_reader.h"

struct BitReader::Internals
{
    const uint8_t *buffer;
    size_t buffer_size;
    uint8_t mask;
    uint8_t value;

    Internals(const uint8_t *buffer, size_t buffer_size) :
        buffer(buffer), buffer_size(buffer_size)
    {
        mask = 0;
        value = 0;
    }

    bool get()
    {
        mask >>= 1;
        if (mask == 0x00)
        {
            if (buffer_size == 0)
                throw std::runtime_error("Trying to read bits beyond EOF");

            mask = 0x80;
            value = *buffer ++;
            -- buffer_size;
        }
        return (value & mask) != 0;
    }

    bool try_get()
    {
        mask >>= 1;
        if (mask == 0x00)
        {
            if (buffer_size == 0)
                return 0;

            mask = 0x80;
            value = *buffer ++;
            -- buffer_size;
        }
        return (value & mask) != 0;
    }
};

BitReader::BitReader(const char *buffer, size_t buffer_size)
    : internals(new Internals(
        reinterpret_cast<const uint8_t*>(buffer), buffer_size))
{
}

BitReader::~BitReader()
{
}

unsigned int BitReader::get(size_t n)
{
    if (n > 32)
        throw std::runtime_error("Too many bits");

    unsigned int value = 0;
    while (n --)
    {
        value <<= 1;
        value |= internals->get();
    }
    return value;
}

unsigned int BitReader::try_get(size_t n)
{
    if (n > 32)
        throw std::runtime_error("Too many bits");

    unsigned int value = 0;
    while (n --)
    {
        value <<= 1;
        value |= internals->try_get();
    }
    return value;
}
