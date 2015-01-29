#ifndef IO_H
#define IO_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct IO IO;

IO *io_create_from_file(const char *path, const char *read_mode);
IO *io_create_from_buffer(const char *buffer, size_t buffer_size);
void io_destroy(IO *io);

size_t io_size(IO *io);
void io_seek(IO *io, size_t offset);
void io_skip(IO *io, size_t offset);
size_t io_tell(IO *io);

char *io_read_until_zero(IO *io);
char *io_read_string(IO *io, size_t length);
uint8_t io_read_u8(IO *io);
uint16_t io_read_u16_le(IO *io);
uint32_t io_read_u32_le(IO *io);
uint64_t io_read_u64_le(IO *io);
uint16_t io_read_u16_be(IO *io);
uint32_t io_read_u32_be(IO *io);
uint64_t io_read_u64_be(IO *io);

bool io_write_string(IO *io, const char *str, size_t length);
bool io_write_u8(IO *io, uint8_t value);
bool io_write_u16_le(IO *io, uint16_t value);
bool io_write_u32_le(IO *io, uint32_t value);
bool io_write_u64_le(IO *io, uint64_t value);
bool io_write_u16_be(IO *io, uint16_t value);
bool io_write_u32_be(IO *io, uint32_t value);
bool io_write_u64_be(IO *io, uint64_t value);

#endif
