#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "io.h"
#include "logger.h"

struct IO
{
    // for the file implementation
    FILE *file;

    // for the buffer implementation
    char *buffer;
    size_t buffer_pos;
    size_t buffer_size;

    bool (*seek)(struct IO *, size_t, int);
    bool (*read)(struct IO *, size_t, void *);
    bool (*write)(struct IO *, size_t, void *);
    size_t (*tell)(struct IO *);
    size_t (*size)(struct IO *);
};



static bool file_io_seek(IO *io, size_t offset, int whence)
{
    assert_not_null(io);
    return fseek(io->file, offset, whence) == 0;
}

static bool file_io_read(IO *io, size_t length, void *destination)
{
    assert_not_null(io);
    if (fread(destination, 1, length, io->file) != length)
    {
        log_warning("Failed to read full data");
        return false;
    }
    return true;
}

static bool file_io_write(IO *io, size_t length, void *source)
{
    assert_not_null(io);
    if (fwrite(source, 1, length, io->file) != length)
    {
        log_warning("Failed to write full data");
        return false;
    }
    return true;
}

static size_t file_io_tell(IO *io)
{
    assert_not_null(io);
    return ftell(io->file);
}

static size_t file_io_size(IO *io)
{
    size_t size;
    size_t old_pos;
    assert_not_null(io);
    old_pos = ftell(io->file);
    fseek(io->file, 0, SEEK_END);
    size = ftell(io->file);
    fseek(io->file, old_pos, SEEK_SET);
    return size;
}



static bool buffer_io_seek(IO *io, size_t offset, int whence)
{
    assert_not_null(io);
    if (whence == SEEK_SET)
        io->buffer_pos = offset;
    else if (whence == SEEK_END)
        io->buffer_pos = io->buffer_size - 1 - offset;
    else if (whence == SEEK_CUR)
        io->buffer_pos += offset;
    else
        assert_that(1 == 0);
    return io->buffer_pos < io->buffer_size;
}

static bool buffer_io_read(IO *io, size_t length, void *destination)
{
    assert_not_null(io);
    assert_that(io->buffer_pos + length <= io->buffer_size);
    memcpy(destination, io->buffer + io->buffer_pos, length);
    io->buffer_pos += length;
    return true;
}

static bool buffer_io_write(IO *io, size_t length, void *source)
{
    char *destination;
    assert_not_null(io);
    if (io->buffer_pos + length > io->buffer_size)
    {
        destination = (char*)realloc(io->buffer, io->buffer_pos + length);
        if (!destination)
        {
            log_error("Failed to allocate memory");
            return false;
        }
        io->buffer_size = io->buffer_pos + length;
        io->buffer = destination;
    }
    destination = io->buffer + io->buffer_pos;
    memcpy(destination, source, length);
    io->buffer_pos += length;
    return true;
}

static size_t buffer_io_tell(IO *io)
{
    assert_not_null(io);
    return io->buffer_pos;
}

static size_t buffer_io_size(IO *io)
{
    return io->buffer_size;
}



IO *io_create_from_file(const char *path, const char *read_mode)
{
    FILE *fp = fopen(path, read_mode);
    if (!fp)
    {
        log_error("Can\'t open file %s", path);
        return NULL;
    }
    IO *io = (IO*)malloc(sizeof(IO));
    assert_not_null(io);
    io->file = fp;
    io->buffer = NULL;
    io->buffer_pos = 0;
    io->buffer_size = 0;
    io->seek = &file_io_seek;
    io->read = &file_io_read;
    io->write = &file_io_write;
    io->tell = &file_io_tell;
    io->size = &file_io_size;
    return io;
}

IO *io_create_from_buffer(const char *buffer, size_t buffer_size)
{
    IO *io = (IO*)malloc(sizeof(IO));
    assert_not_null(io);
    io->file = NULL;
    io->buffer = (char*)malloc(buffer_size);
    assert_not_null(io->buffer);
    memcpy(io->buffer, buffer, buffer_size);
    io->buffer_pos = 0;
    io->buffer_size = buffer_size;
    io->seek = &buffer_io_seek;
    io->read = &buffer_io_read;
    io->write = &buffer_io_write;
    io->tell = &buffer_io_tell;
    io->size = &buffer_io_size;
    return io;
}

IO *io_create_empty()
{
    return io_create_from_buffer("", 0);
}

void io_destroy(IO *io)
{
    assert_not_null(io);
    if (io->file != NULL)
        fclose(io->file);
    if (io->buffer != NULL)
        free(io->buffer);
}

size_t io_size(IO *io)
{
    assert_not_null(io);
    assert_that(io->size != NULL);
    return io->size(io);
}

bool io_seek(IO *io, size_t offset)
{
    assert_not_null(io);
    assert_that(io->seek != NULL);
    return io->seek(io, offset, SEEK_SET);
}

bool io_skip(IO *io, size_t offset)
{
    assert_not_null(io);
    assert_that(io->seek != NULL);
    return io->seek(io, offset, SEEK_CUR);
}

size_t io_tell(IO *io)
{
    assert_not_null(io);
    assert_that(io->tell != NULL);
    return io->tell(io);
}

bool io_read_string(IO *io, char *output, size_t length)
{
    assert_not_null(io);
    assert_not_null(output);
    assert_that(io->read != NULL);
    return io->read(io, length, output);
}

bool io_read_until_zero(IO *io, char **output, size_t *output_size)
{
    char *new_str;
    char c;
    assert_not_null(io);
    assert_not_null(output);
    *output = NULL;
    size_t size = 0;
    do
    {
        new_str = (char*)realloc(*output, size + 1);
        if (new_str == NULL)
        {
            free(*output);
            *output = NULL;
            *output_size = 0;
            log_error("Failed to allocate memory");
            return false;
        }
        *output = new_str;
        c = io_read_u8(io);
        (*output)[size ++] = c;
    }
    while (c != '\0');
    if (output_size != NULL)
        *output_size = size;
    return true;
}

uint8_t io_read_u8(IO *io)
{
    uint8_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 1, &ret);
    return ret;
}

uint16_t io_read_u16_le(IO *io)
{
    uint16_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 2, &ret);
    return le16toh(ret);
}

uint32_t io_read_u32_le(IO *io)
{
    uint32_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 4, &ret);
    return le32toh(ret);
}

uint64_t io_read_u64_le(IO *io)
{
    uint64_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 8, &ret);
    return le64toh(ret);
}

uint16_t io_read_u16_be(IO *io)
{
    uint16_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 2, &ret);
    return be16toh(ret);
}

uint32_t io_read_u32_be(IO *io)
{
    uint32_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 4, &ret);
    return be32toh(ret);
}

uint64_t io_read_u64_be(IO *io)
{
    uint64_t ret = 0;
    assert_not_null(io);
    assert_that(io->read != NULL);
    io->read(io, 8, &ret);
    return be64toh(ret);
}

bool io_write_string(IO *io, const char *str, size_t length)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    return io->write(io, length, (void*)str);
}

bool io_write_u8(IO *io, uint8_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    return io->write(io, 1, &value);
}

bool io_write_u16_le(IO *io, uint16_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    value = le16toh(value);
    return io->write(io, 2, &value);
}

bool io_write_u32_le(IO *io, uint32_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    value = le32toh(value);
    return io->write(io, 4, &value);
}

bool io_write_u64_le(IO *io, uint64_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    value = le64toh(value);
    return io->write(io, 8, &value);
}

bool io_write_u16_be(IO *io, uint16_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    value = be16toh(value);
    return io->write(io, 2, &value);
}

bool io_write_u32_be(IO *io, uint32_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    value = be32toh(value);
    return io->write(io, 4, &value);
}

bool io_write_u64_be(IO *io, uint64_t value)
{
    assert_not_null(io);
    assert_that(io->write != NULL);
    value = be64toh(value);
    return io->write(io, 8, &value);
}
