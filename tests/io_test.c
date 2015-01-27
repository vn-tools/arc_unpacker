#include <stdlib.h>
#include "io.h"
#include "assert.h"

void test_file_simple_read()
{
    IO *io = io_create_from_file("tests/test_files/reimu_transparent.png", "rb");
    char *png_magic = io_read_string(io, 4);
    assert_equals("\x89PNG", png_magic);
    free(png_magic);
    io_destroy(io);
}

void test_buffer_empty()
{
    IO *io = io_create_from_buffer("", 0);
    assert_equali(0, io_size(io));
    assert_equali(0, io_tell(io));
    io_destroy(io);
}

void test_buffer_simple_read()
{
    IO *io = io_create_from_buffer("\x01\x00\x00\x00", 4);
    assert_equali(1, io_read_u32(io));
    assert_equali(4, io_size(io));
    io_destroy(io);
}

void test_buffer_skip_and_tell()
{
    IO *io = io_create_from_buffer("\x01\x0f\x00\x00", 4);
    io_skip(io, 1);
    assert_equali(1, io_tell(io));
    assert_equali(15, io_read_u16(io));
    assert_equali(3, io_tell(io));
    io_destroy(io);
}

void test_buffer_seek_and_tell()
{
    IO *io = io_create_from_buffer("\x01\x00\x00\x00", 4);
    assert_equali(1, io_read_u32(io));
    io_seek(io, 0);
    assert_equali(0, io_tell(io));
    assert_equali(1, io_read_u32(io));
    io_seek(io, 2);
    assert_equali(2, io_tell(io));
    io_destroy(io);
}

void test_read_until_zero()
{
    IO *io = io_create_from_buffer("abc\x00", 4);
    char *result = io_read_until_zero(io);
    assert_equals("abc", result);
    free(result);
    io_destroy(io);
}

void test_read_string()
{
    IO *io = io_create_from_buffer("abc\x00", 4);
    char *result = io_read_string(io, 2);
    assert_equals("ab", result);
    free(result);
    io_destroy(io);
}

int main(void)
{
    test_file_simple_read();
    test_buffer_empty();
    test_buffer_simple_read();
    test_buffer_skip_and_tell();
    test_buffer_seek_and_tell();
    test_read_until_zero();
    test_read_string();
    return 0;
}
