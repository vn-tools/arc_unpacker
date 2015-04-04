#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "test_support/eassert.h"

void test_empty()
{
    BitReader reader("", 0);
    try
    {
        reader.get(1);
        eassert(0);
    }
    catch (...)
    {
    }
}

void test_reading_one_bit()
{
    BitReader reader("\x8f", 2); //10001111
    eassert(reader.get(1));
    eassert(!reader.get(1));
    eassert(!reader.get(1));
    eassert(!reader.get(1));
    eassert(reader.get(1));
    eassert(reader.get(1));
    eassert(reader.get(1));
    eassert(reader.get(1));
}

void test_reading_many_bits()
{
    BitReader reader("\x8f", 2); //10001111
    eassert(reader.get(7) == (0x8f >> 1));
    eassert(reader.get(1));
}

void test_reading_many_bytes()
{
    BitReader reader("\x8f\x8f", 2); //10001111
    eassert(reader.get(7) == (0x8f >> 1));
    eassert(reader.get(1));

    eassert(reader.get(1));
    eassert(!reader.get(1));
    eassert(reader.get(4) == 3);
    eassert(reader.get(2) == 3);
    try
    {
        reader.get(1);
        eassert(0);
    }
    catch (...)
    {
    }
}

int main(void)
{
    test_empty();
    test_reading_one_bit();
    test_reading_many_bits();
    test_reading_many_bytes();
    return 0;
}
