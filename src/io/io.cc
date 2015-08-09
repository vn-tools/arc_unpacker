#include <memory>
#include "util/endian.h"
#include "util/range.h"
#include "io/io.h"

using namespace au;
using namespace au::io;

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

bstr IO::read(size_t bytes)
{
    bstr ret;
    ret.resize(bytes);
    read(&ret[0], bytes);
    return ret;
}

bstr IO::read_until_zero()
{
    bstr output;
    char c;
    while ((c = read_u8()) != '\0')
        output += c;
    return output;
}

bstr IO::read_until_zero(size_t bytes)
{
    bstr output = read(bytes);
    for (auto i : util::range(output.size()))
        if (!output[i])
            return output.substr(0, i);
    return output;
}

bstr IO::read_until_end()
{
    return read(size() - tell());
}

bstr IO::read_line()
{
    bstr output;
    char c;
    while (!eof())
    {
        c = read_u8();
        if (c == '\0' || c == '\n')
            break;
        if (c != '\r')
            output += c;
    }
    return output;
}

u8 IO::read_u8()
{
    u8 ret = 0;
    read(&ret, 1);
    return ret;
}

u16 IO::read_u16_le()
{
    u16 ret = 0;
    read(&ret, 2);
    return util::from_little_endian<u16>(ret);
}

u32 IO::read_u32_le()
{
    u32 ret = 0;
    read(&ret, 4);
    return util::from_little_endian<u32>(ret);
}

u64 IO::read_u64_le()
{
    u64 ret = 0;
    read(&ret, 8);
    return util::from_little_endian<u64>(ret);
}

u16 IO::read_u16_be()
{
    u16 ret = 0;
    read(&ret, 2);
    return util::from_big_endian<u16>(ret);
}

u32 IO::read_u32_be()
{
    u32 ret = 0;
    read(&ret, 4);
    return util::from_big_endian<u32>(ret);
}

u64 IO::read_u64_be()
{
    u64 ret = 0;
    read(&ret, 8);
    return util::from_big_endian<u64>(ret);
}

void IO::write_from_io(IO &input)
{
    write_from_io(input, input.size() - input.tell());
}

void IO::write(const bstr &bytes)
{
    write(bytes.get<char>(), bytes.size());
}

void IO::write_u8(u8 value)
{
    write(&value, 1);
}

void IO::write_u16_le(u16 value)
{
    value = util::to_little_endian<u16>(value);
    write(&value, 2);
}

void IO::write_u32_le(u32 value)
{
    value = util::to_little_endian<u32>(value);
    write(&value, 4);
}

void IO::write_u64_le(u64 value)
{
    value = util::to_little_endian<u64>(value);
    write(&value, 8);
}

void IO::write_u16_be(u16 value)
{
    value = util::to_big_endian<u16>(value);
    write(&value, 2);
}

void IO::write_u32_be(u32 value)
{
    value = util::to_big_endian<u32>(value);
    write(&value, 4);
}

void IO::write_u64_be(u64 value)
{
    value = util::to_big_endian<u64>(value);
    write(&value, 8);
}
