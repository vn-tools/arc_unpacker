#include <stdio.h>
#include <stdlib.h>
#include "assert_ex.h"
#include "io.h"

void test_file_simple_read()
{
    IO *io = io_create_from_file(
        "tests/test_files/gfx/reimu_transparent.png",
        "rb");
    char png_magic[4];
    io_read_string(io, png_magic, 4);
    assert_equalsn("\x89PNG", png_magic, 4);
    io_destroy(io);
}

void test_file_simple_write()
{
    IO *io = io_create_from_file("tests/test_files/trash.out", "w+b");
    assert_that(io_write_u32_le(io, 1));
    io_seek(io, 0);
    assert_equali(1, io_read_u32_le(io));
    assert_equali(4, io_size(io));
    io_destroy(io);
    remove("tests/test_files/trash.out");
}

void test_buffer_empty()
{
    IO *io = io_create_from_buffer("", 0);
    assert_equali(0, io_size(io));
    assert_equali(0, io_tell(io));
    io_destroy(io);
}

void test_buffer_binary_data()
{
    IO *io = io_create_from_buffer("\x00\x00\x00\x01", 4);
    assert_equali(4, io_size(io));
    assert_equali(0x01000000, io_read_u32_le(io));
    io_seek(io, 0);
    io_write_string(io, "\x00\x00\x00\x02", 4);
    io_seek(io, 0);
    assert_equali(0x02000000, io_read_u32_le(io));
    io_destroy(io);
}

void test_buffer_simple_read()
{
    IO *io = io_create_from_buffer("\x01\x00\x00\x00", 4);
    assert_equali(1, io_read_u32_le(io));
    assert_equali(4, io_size(io));
    io_destroy(io);
}

void test_buffer_simple_write()
{
    IO *io = io_create_from_buffer("", 0);
    io_write_u32_le(io, 1);
    io_seek(io, 0);
    assert_equali(1, io_read_u32_le(io));
    assert_equali(4, io_size(io));
    io_destroy(io);
}

void test_buffer_skip_and_tell()
{
    IO *io = io_create_from_buffer("\x01\x0f\x00\x00", 4);
    io_skip(io, 1);
    assert_equali(1, io_tell(io));
    assert_equali(15, io_read_u16_le(io));
    assert_equali(3, io_tell(io));
    io_destroy(io);
}

void test_buffer_seek_and_tell()
{
    IO *io = io_create_from_buffer("\x01\x00\x00\x00", 4);
    assert_equali(1, io_read_u32_le(io));
    io_seek(io, 0);
    assert_equali(0, io_tell(io));
    assert_equali(1, io_read_u32_le(io));
    io_seek(io, 2);
    assert_equali(2, io_tell(io));
    io_destroy(io);
}

void test_read_until_zero()
{
    IO *io = io_create_from_buffer("abc\x00", 4);
    char *result = NULL;
    size_t length = 0;
    io_read_until_zero(io, &result, &length);
    assert_equals("abc", result);
    assert_equali(4, length);
    free(result);
    io_destroy(io);
}

void test_read_string()
{
    IO *io = io_create_from_buffer("abc\x00", 4);
    char result[2];
    io_read_string(io, result, 2);
    assert_equalsn("ab", result, 2);
    io_destroy(io);
}

void test_write_string()
{
    IO *io = io_create_from_buffer("abc\x00", 4);
    io_write_string(io, "xy", 2);
    io_skip(io, -2);
    char result[3];
    io_read_string(io, result, 3);
    assert_equalsn("xyc", result, 3);
    io_destroy(io);
}

void test_endianness()
{
    IO *io = io_create_from_buffer("\x12\x34\x56\x78", 4);
    assert_equali(0x12, io_read_u8(io)); io_skip(io, -1);
    assert_equali(0x3412, io_read_u16_le(io)); io_skip(io, -2);
    assert_equali(0x1234, io_read_u16_be(io)); io_skip(io, -2);
    assert_equali(0x78563412, io_read_u32_le(io)); io_skip(io, -4);
    assert_equali(0x12345678, io_read_u32_be(io)); io_skip(io, -4);
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
