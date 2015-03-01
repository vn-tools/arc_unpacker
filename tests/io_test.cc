#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "file_io.h"
#include "buffered_io.h"
#include "test_support/eassert.h"

void test_file_simple_read()
{
    FileIO io("tests/files/reimu_transparent.png", "rb");
    char png_magic[4];
    io.read(png_magic, 4);
    eassert(memcmp("\x89PNG", png_magic, 4) == 0);
}

void test_file_simple_write()
{
    std::unique_ptr<FileIO> io(new FileIO("tests/files/trash.out", "w+b"));
    eassert(io != nullptr);
    io->write_u32_le(1);
    io->seek(0);
    eassert(io->read_u32_le() == 1);
    eassert(io->size() == 4);
    remove("tests/files/trash.out");
}

void test_buffer_empty()
{
    BufferedIO io;
    eassert(io.size() == 0);
    eassert(io.tell() == 0);
}

void test_buffer_binary_data()
{
    BufferedIO io("\x00\x00\x00\x01", 4);
    eassert(io.size() == 4);
    eassert(io.read_u32_le() == 0x01000000);
    io.seek(0);
    io.write("\x00\x00\x00\x02", 4);
    io.seek(0);
    eassert(io.read_u32_le() == 0x02000000);
}

void test_buffer_simple_read()
{
    BufferedIO io("\x01\x00\x00\x00", 4);
    eassert(io.read_u32_le() == 1);
    eassert(io.size() == 4);
}

void test_buffer_simple_write()
{
    BufferedIO io;
    io.write_u32_le(1);
    io.seek(0);
    eassert(io.read_u32_le() == 1);
    eassert(io.size() == 4);
}

void test_buffer_skip_and_tell()
{
    BufferedIO io("\x01\x0f\x00\x00", 4);
    io.skip(1);
    eassert(io.tell() == 1);
    eassert(io.read_u16_le() == 15);
    eassert(io.tell() == 3);
}

void test_buffer_seek_and_tell()
{
    BufferedIO io("\x01\x00\x00\x00", 4);

    eassert(io.tell() == 0);

    io.seek(4);
    eassert(io.tell() == 4);

    try
    {
        io.seek(5);
        eassert(0);
    }
    catch (...)
    {
        eassert(io.tell() == 4);
    }

    io.seek(0);
    eassert(io.read_u32_le() == 1);

    io.seek(0);
    eassert(io.tell() == 0);
    eassert(io.read_u32_le() == 1);
    io.seek(2);
    eassert(io.tell() == 2);

    io.skip(1);
    eassert(io.tell() == 3);

    io.skip(-1);
    eassert(io.tell() == 2);
}

void test_read_until_zero()
{
    BufferedIO io("abc\x00", 4);
    char *result = nullptr;
    size_t length = 0;
    io.read_until_zero(&result, &length);
    eassert(strcmp("abc", result) == 0);
    eassert(length == length);
    free(result);
}

void test_read_string()
{
    BufferedIO io("abc\x00", 4);
    char result[2];
    io.read(result, 2);
    eassert(memcmp("ab", result, 2) == 0);
}

void test_write_string()
{
    BufferedIO io("abc\x00", 4);
    io.write("xy", 2);
    io.skip(-2);
    char result[3];
    io.read(result, 3);
    eassert(memcmp("xyc", result, 3) == 0);
}

void test_endianness()
{
    BufferedIO io("\x12\x34\x56\x78", 4);
    eassert(io.read_u8() == 0x12); io.skip(-1);
    eassert(io.read_u16_le() == 0x3412); io.skip(-2);
    eassert(io.read_u16_be() == 0x1234); io.skip(-2);
    eassert(io.read_u32_le() == 0x78563412); io.skip(-4);
    eassert(io.read_u32_be() == 0x12345678); io.skip(-4);
}

int main(void)
{
    test_file_simple_read();
    test_file_simple_write();
    test_buffer_empty();
    test_buffer_binary_data();
    test_buffer_simple_read();
    test_buffer_simple_write();
    test_buffer_skip_and_tell();
    test_buffer_seek_and_tell();
    test_read_until_zero();
    test_read_string();
    test_write_string();
    test_endianness();
    return 0;
}
