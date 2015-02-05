#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "endian.h"
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
    bool (*truncate)(struct IO *, size_t);
};



static bool file_io_seek(IO *io, size_t offset, int whence)
{
    assert(io != NULL);
    return fseek(io->file, offset, whence) == 0;
}

static bool file_io_read(IO *io, size_t length, void *destination)
{
    assert(io != NULL);
    assert(destination != NULL);
    if (fread(destination, 1, length, io->file) != length)
    {
        log_warning("IO: Failed to read full data");
        return false;
    }
    return true;
}

static bool file_io_write(IO *io, size_t length, void *source)
{
    assert(io != NULL);
    assert(source != NULL);
    if (fwrite(source, 1, length, io->file) != length)
    {
        log_warning("IO: Failed to write full data");
        return false;
    }
    return true;
}

static size_t file_io_tell(IO *io)
{
    assert(io != NULL);
    return ftell(io->file);
}

static size_t file_io_size(IO *io)
{
    assert(io != NULL);
    size_t old_pos = ftell(io->file);
    fseek(io->file, 0, SEEK_END);
    size_t size = ftell(io->file);
    fseek(io->file, old_pos, SEEK_SET);
    return size;
}

static bool file_io_truncate(
    __attribute__((unused)) IO *io,
    __attribute__((unused)) size_t new_size)
{
    assert(io != NULL);
    //return ftruncate(io->file, new_size) == 0;
    log_error("IO: Truncating for real files is not supported!");
    return false;
}



static bool buffer_io_seek(IO *io, size_t offset, int whence)
{
    assert(io != NULL);
    assert(whence == SEEK_SET || whence == SEEK_END || whence == SEEK_CUR);

    if (whence == SEEK_SET)
        io->buffer_pos = offset;
    else if (whence == SEEK_END)
        io->buffer_pos = io->buffer_size - 1 - offset;
    else if (whence == SEEK_CUR)
        io->buffer_pos += offset;

    return io->buffer_pos < io->buffer_size;
}

static bool buffer_io_read(IO *io, size_t length, void *destination)
{
    assert(io != NULL);
    assert(destination != NULL);
    if (io->buffer_pos + length > io->buffer_size)
        return false;
    memcpy(destination, io->buffer + io->buffer_pos, length);
    io->buffer_pos += length;
    return true;
}

static bool buffer_io_write(IO *io, size_t length, void *source)
{
    assert(io != NULL);
    assert(source != NULL);

    char *destination;
    if (io->buffer_pos + length > io->buffer_size)
    {
        destination = (char*)realloc(io->buffer, io->buffer_pos + length);
        if (destination == NULL)
        {
            log_error("IO: Failed to allocate %d bytes",
                io->buffer_pos + length);
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
    assert(io != NULL);
    return io->buffer_pos;
}

static size_t buffer_io_size(IO *io)
{
    assert(io != NULL);
    return io->buffer_size;
}

static bool buffer_io_truncate(IO *io, size_t new_size)
{
    assert(io != NULL);
    if (new_size == 0)
    {
        free(io->buffer);
        io->buffer_size = 0;
        io->buffer_pos = 0;
        io->buffer = NULL;
        return true;
    }
    char *new_buffer = (char*)realloc(io->buffer, new_size);
    if (new_buffer == NULL)
        return false;
    io->buffer = new_buffer;
    io->buffer_size = new_size;
    if (io->buffer_pos >= new_size)
        io->buffer_pos = new_size;
    return true;
}



IO *io_create_from_file(const char *path, const char *read_mode)
{
    FILE *fp = fopen(path, read_mode);
    if (!fp)
    {
        log_error("IO: Can\'t open file %s", path);
        return NULL;
    }
    IO *io = new IO;
    if (io == NULL)
    {
        log_error("IO: Failed to allocate memory for file IO");
        return NULL;
    }
    io->file = fp;
    io->buffer = NULL;
    io->buffer_pos = 0;
    io->buffer_size = 0;
    io->seek = &file_io_seek;
    io->read = &file_io_read;
    io->write = &file_io_write;
    io->tell = &file_io_tell;
    io->size = &file_io_size;
    io->truncate = &file_io_truncate;
    return io;
}

IO *io_create_from_buffer(const char *buffer, size_t buffer_size)
{
    IO *io = new IO;
    if (io == NULL)
    {
        log_error("IO: Failed to allocate memory for buffer IO");
        return NULL;
    }
    io->file = NULL;
    io->buffer = (char*)malloc(buffer_size);
    if (io->buffer == NULL)
    {
        log_error(
            "IO: Failed to allocate %d bytes for buffer IO data",
            buffer_size);
        delete io;
        return NULL;
    }
    memcpy(io->buffer, buffer, buffer_size);
    io->buffer_pos = 0;
    io->buffer_size = buffer_size;
    io->seek = &buffer_io_seek;
    io->read = &buffer_io_read;
    io->write = &buffer_io_write;
    io->tell = &buffer_io_tell;
    io->size = &buffer_io_size;
    io->truncate = &buffer_io_truncate;
    return io;
}

IO *io_create_empty()
{
    return io_create_from_buffer("", 0);
}

void io_destroy(IO *io)
{
    assert(io != NULL);
    if (io->file != NULL)
        fclose(io->file);
    if (io->buffer != NULL)
        free(io->buffer);
    delete io;
}

size_t io_size(IO *io)
{
    assert(io != NULL);
    assert(io->size != NULL);
    return io->size(io);
}

bool io_seek(IO *io, size_t offset)
{
    assert(io != NULL);
    assert(io->seek != NULL);
    return io->seek(io, offset, SEEK_SET);
}

bool io_skip(IO *io, size_t offset)
{
    assert(io != NULL);
    assert(io->seek != NULL);
    return io->seek(io, offset, SEEK_CUR);
}

size_t io_tell(IO *io)
{
    assert(io != NULL);
    assert(io->tell != NULL);
    return io->tell(io);
}

bool io_read_string(IO *io, char *output, size_t length)
{
    assert(io != NULL);
    assert(output != NULL);
    assert(io->read != NULL);
    return io->read(io, length, output);
}

bool io_read_until_zero(IO *io, char **output, size_t *output_size)
{
    assert(io != NULL);
    assert(output != NULL);

    char *new_str;
    char c;
    *output = NULL;
    size_t size = 0;
    do
    {
        new_str = (char*)realloc(*output, size + 1);
        if (new_str == NULL)
        {
            free(*output);
            *output = NULL;
            if (output_size != NULL)
                *output_size = 0;
            log_error(
                "IO: Failed to allocate memory for null-terminated string");
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
    assert(io != NULL);
    assert(io->read != NULL);
    uint8_t ret = 0;
    io->read(io, 1, &ret);
    return ret;
}

uint16_t io_read_u16_le(IO *io)
{
    assert(io != NULL);
    assert(io->read != NULL);
    uint16_t ret = 0;
    io->read(io, 2, &ret);
    return le16toh(ret);
}

uint32_t io_read_u32_le(IO *io)
{
    assert(io != NULL);
    assert(io->read != NULL);
    uint32_t ret = 0;
    io->read(io, 4, &ret);
    return le32toh(ret);
}

uint64_t io_read_u64_le(IO *io)
{
    assert(io != NULL);
    assert(io->read != NULL);
    uint64_t ret = 0;
    io->read(io, 8, &ret);
    return le64toh(ret);
}

uint16_t io_read_u16_be(IO *io)
{
    assert(io != NULL);
    assert(io->read != NULL);
    uint16_t ret = 0;
    io->read(io, 2, &ret);
    return be16toh(ret);
}

uint32_t io_read_u32_be(IO *io)
{
    assert(io != NULL);
    assert(io->read != NULL);
    uint32_t ret = 0;
    io->read(io, 4, &ret);
    return be32toh(ret);
}

uint64_t io_read_u64_be(IO *io)
{
    assert(io != NULL);
    assert(io->read != NULL);
    uint64_t ret = 0;
    io->read(io, 8, &ret);
    return be64toh(ret);
}

bool io_write_string(IO *io, const char *str, size_t length)
{
    assert(io != NULL);
    assert(io->write != NULL);
    assert(str != NULL);
    return io->write(io, length, (void*)str);
}

bool io_write_string_from_io(IO *io, IO *input, size_t length)
{
    assert(io != NULL);
    assert(input != NULL);

    bool result;
    if (io->file != NULL)
    {
        // TODO improvement: use static buffer instead of such allocation
        char *buffer = new char[length];
        if (buffer == NULL)
        {
            log_error("IO: Failed to allocate memory for string");
            return NULL;
        }
        result = io_read_string(input, buffer, length);
        if (result)
            result &= io_write_string(io, buffer, length);
        delete []buffer;
    }
    else
    {
        size_t new_pos = io->buffer_pos + length;
        if (new_pos > io->buffer_size)
        {
            char *new_buffer = (char*)realloc(io->buffer, new_pos);
            if (new_buffer == NULL)
            {
                log_error(
                    "IO: Failed to reallocate memory for string to %d bytes",
                    new_pos);
            }
            io->buffer = new_buffer;
            io->buffer_size = new_pos;
        }
        result = io_read_string(input, io->buffer + io->buffer_pos, length);
        io->buffer_pos = new_pos;
    }

    return result;
}

bool io_write_u8(IO *io, uint8_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    return io->write(io, 1, &value);
}

bool io_write_u16_le(IO *io, uint16_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    value = htole16(value);
    return io->write(io, 2, &value);
}

bool io_write_u32_le(IO *io, uint32_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    value = htole32(value);
    return io->write(io, 4, &value);
}

bool io_write_u64_le(IO *io, uint64_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    value = htole64(value);
    return io->write(io, 8, &value);
}

bool io_write_u16_be(IO *io, uint16_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    value = htobe16(value);
    return io->write(io, 2, &value);
}

bool io_write_u32_be(IO *io, uint32_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    value = htobe32(value);
    return io->write(io, 4, &value);
}

bool io_write_u64_be(IO *io, uint64_t value)
{
    assert(io != NULL);
    assert(io->write != NULL);
    value = htobe64(value);
    return io->write(io, 8, &value);
}

bool io_truncate(IO *io, size_t new_size)
{
    assert(io != NULL);
    assert(io->truncate != NULL);
    return io->truncate(io, new_size);
}
