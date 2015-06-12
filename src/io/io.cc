#include <memory>
#include "util/endian.h"
#include "io/io.h"

IO::~IO()
{
}

void IO::peek(size_t offset, std::function<void()> func)
{
    size_t old_pos = tell();
    seek(offset);
    try
    {
        func();
        seek(old_pos);
    }
    catch (...)
    {
        seek(old_pos);
        throw;
    }
}

bool IO::eof() const
{
    return tell() == size();
}

std::string IO::read_until_zero()
{
    std::string output;
    char c;
    while ((c = read_u8()) != '\0')
        output.push_back(c);
    return output;
}

std::string IO::read_until_zero(size_t bytes)
{
    std::string output = read(bytes);
    size_t pos = output.find('\x00');
    if (pos != std::string::npos)
        output = output.substr(0, pos);
    return output;
}

std::string IO::read_line()
{
    std::string output;
    char c;
    while (!eof())
    {
        c = read_u8();
        if (c == '\0' || c == '\n')
            break;
        if (c != '\r')
            output.push_back(c);
    }
    return output;
}

std::string IO::read_until_end()
{
    return read(size() - tell());
}

std::string IO::read(size_t bytes)
{
    std::unique_ptr<char[]> buffer(new char[bytes]);
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

void IO::write_from_io(IO &input)
{
    write_from_io(input, input.size() - input.tell());
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
