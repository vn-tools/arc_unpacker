#include <cassert>
#include "bit_reader.h"
#include "buffered_io.h"

void test_empty()
{
    BufferedIO io;
    BitReader reader(io);
    try
    {
        reader.get();
        assert(0);
    }
    catch (...)
    {
    }
}

void test_reading_one_bit()
{
    BufferedIO io("\x8f"); //10001111
    BitReader reader(io);
    assert(reader.get());
    assert(!reader.get());
    assert(!reader.get());
    assert(!reader.get());
    assert(reader.get());
    assert(reader.get());
    assert(reader.get());
    assert(reader.get());
}

void test_reading_many_bits()
{
    BufferedIO io("\x8f"); //10001111
    BitReader reader(io);
    assert(reader.get(7) == (0x8f >> 1));
    assert(reader.get());
}

int main(void)
{
    test_empty();
    test_reading_one_bit();
    return 0;
}
