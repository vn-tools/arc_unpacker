#include "io/stream.h"
#include <memory>
#include "util/endian.h"
#include "util/range.h"

using namespace au;
using namespace au::io;

Stream::~Stream()
{
}

Stream &Stream::peek(size_t offset, std::function<void()> func)
{
    size_t old_pos = tell();
    seek(offset);
    try
    {
        func();
        seek(old_pos);
        return *this;
    }
    catch (...)
    {
        seek(old_pos);
        throw;
    }
}

bool Stream::eof() const
{
    return tell() == size();
}

bstr Stream::read(size_t bytes)
{
    if (!bytes)
        return ""_b;
    bstr ret(bytes);
    read_impl(&ret[0], bytes);
    return ret;
}

bstr Stream::read_to_zero()
{
    bstr output;
    char c;
    while ((c = read_u8()) != '\0')
        output += c;
    return output;
}

bstr Stream::read_to_zero(size_t bytes)
{
    bstr output = read(bytes);
    for (auto i : util::range(output.size()))
        if (!output[i])
            return output.substr(0, i);
    return output;
}

bstr Stream::read_to_eof()
{
    return read(size() - tell());
}

bstr Stream::read_line()
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

u8 Stream::read_u8()
{
    u8 ret = 0;
    read_impl(&ret, 1);
    return ret;
}

u16 Stream::read_u16_le()
{
    u16 ret = 0;
    read_impl(&ret, 2);
    return util::from_little_endian<u16>(ret);
}

u32 Stream::read_u32_le()
{
    u32 ret = 0;
    read_impl(&ret, 4);
    return util::from_little_endian<u32>(ret);
}

u64 Stream::read_u64_le()
{
    u64 ret = 0;
    read_impl(&ret, 8);
    return util::from_little_endian<u64>(ret);
}

u16 Stream::read_u16_be()
{
    u16 ret = 0;
    read_impl(&ret, 2);
    return util::from_big_endian<u16>(ret);
}

u32 Stream::read_u32_be()
{
    u32 ret = 0;
    read_impl(&ret, 4);
    return util::from_big_endian<u32>(ret);
}

u64 Stream::read_u64_be()
{
    u64 ret = 0;
    read_impl(&ret, 8);
    return util::from_big_endian<u64>(ret);
}

Stream &Stream::write(const bstr &bytes)
{
    if (!bytes.size())
        return *this;
    write_impl(bytes.get<char>(), bytes.size());
    return *this;
}

Stream &Stream::write_u8(u8 value)
{
    write_impl(&value, 1);
    return *this;
}

Stream &Stream::write_u16_le(u16 value)
{
    value = util::to_little_endian<u16>(value);
    write_impl(&value, 2);
    return *this;
}

Stream &Stream::write_u32_le(u32 value)
{
    value = util::to_little_endian<u32>(value);
    write_impl(&value, 4);
    return *this;
}

Stream &Stream::write_u64_le(u64 value)
{
    value = util::to_little_endian<u64>(value);
    write_impl(&value, 8);
    return *this;
}

Stream &Stream::write_u16_be(u16 value)
{
    value = util::to_big_endian<u16>(value);
    write_impl(&value, 2);
    return *this;
}

Stream &Stream::write_u32_be(u32 value)
{
    value = util::to_big_endian<u32>(value);
    write_impl(&value, 4);
    return *this;
}

Stream &Stream::write_u64_be(u64 value)
{
    value = util::to_big_endian<u64>(value);
    write_impl(&value, 8);
    return *this;
}
