#include "bit_reader.h"

struct BitReader::Internals
{
    BufferedIO &io;
    unsigned int mask;
    unsigned char value;

    Internals(BufferedIO &io) : io(io)
    {
        mask = 0;
        value = 0;
    }
};

BitReader::BitReader(BufferedIO &io) : internals(new Internals(io))
{
}

BitReader::~BitReader()
{
}

bool BitReader::get()
{
    internals->mask >>= 1;
    if (internals->mask == 0x00)
    {
        internals->mask = 0x80;
        internals->value = internals->io.read_u8();
    }
    return (internals->value & internals->mask) != 0;
}

unsigned int BitReader::get(size_t n)
{
    if (n > 32)
        throw std::runtime_error("Too many bits");

    unsigned int value = 0;
    while (n --)
    {
        value <<= 1;
        value |= get();
    }
    return value;
}
