#include <cassert>
#include <cstdlib>
#include <memory>
#include "endian.h"
#include "io.h"
#include "logger.h"

std::string IO::read_until_zero()
{
    std::string output;
    char c;
    while ((c = read_u8()) != '\0')
        output.push_back(c);
    return output;
}

bool IO::read_until_zero(char **output, size_t *output_size)
{
    assert(output != nullptr);

    char *new_str;
    char c;
    *output = nullptr;
    size_t size = 0;
    do
    {
        new_str = (char*)realloc(*output, size + 1);
        if (new_str == nullptr)
        {
            free(*output);
            *output = nullptr;
            if (output_size != nullptr)
                *output_size = 0;
            log_error(
                "IO: Failed to allocate memory for null-terminated string");
            return false;
        }
        *output = new_str;
        c = read_u8();
        (*output)[size ++] = c;
    }
    while (c != '\0');
    if (output_size != nullptr)
        *output_size = size;
    return true;
}

std::string IO::read(size_t bytes)
{
    std::unique_ptr<char> buffer(new char[bytes]);
    read(buffer.get(), bytes);
    return std::string(buffer.get(), bytes);
}

uint8_t IO::read_u8()
{
    uint8_t ret = 0;
    read(&ret, 1);
    return ret;
}

uint16_t IO::read_u16_le()
{
    uint16_t ret = 0;
    read(&ret, 2);
    return le16toh(ret);
}

uint32_t IO::read_u32_le()
{
    uint32_t ret = 0;
    read(&ret, 4);
    return le32toh(ret);
}

uint64_t IO::read_u64_le()
{
    uint64_t ret = 0;
    read(&ret, 8);
    return le64toh(ret);
}

uint16_t IO::read_u16_be()
{
    uint16_t ret = 0;
    read(&ret, 2);
    return be16toh(ret);
}

uint32_t IO::read_u32_be()
{
    uint32_t ret = 0;
    read(&ret, 4);
    return be32toh(ret);
}

uint64_t IO::read_u64_be()
{
    uint64_t ret = 0;
    read(&ret, 8);
    return be64toh(ret);
}

void IO::write(const std::string &bytes)
{
    write(bytes.data(), bytes.size());
}

void IO::write_u8(uint8_t value)
{
    write(&value, 1);
}

void IO::write_u16_le(uint16_t value)
{
    value = htole16(value);
    write(&value, 2);
}

void IO::write_u32_le(uint32_t value)
{
    value = htole32(value);
    write(&value, 4);
}

void IO::write_u64_le(uint64_t value)
{
    value = htole64(value);
    write(&value, 8);
}

void IO::write_u16_be(uint16_t value)
{
    value = htobe16(value);
    write(&value, 2);
}

void IO::write_u32_be(uint32_t value)
{
    value = htobe32(value);
    write(&value, 4);
}

void IO::write_u64_be(uint64_t value)
{
    value = htobe64(value);
    write(&value, 8);
}
