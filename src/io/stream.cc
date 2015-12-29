#include "io/stream.h"
#include <algorithm>
#include <memory>
#include "algo/endian.h"
#include "algo/range.h"

using namespace au;
using namespace au::io;

template<typename T, const algo::Endianness endianness> inline T
    read_any_primitive(Stream &input_stream)
{
    bstr tmp = input_stream.read(sizeof(T));
    if (endianness != algo::get_machine_endianness())
        std::reverse(tmp.begin(), tmp.end());
    return *reinterpret_cast<const T*>(tmp.get<const char>());
}

Stream &Stream::peek(const size_t offset, std::function<void()> func)
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

bstr Stream::read(const size_t bytes)
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
    while (!eof())
    {
        char c = read_u8();
        if (c == '\0')
            break;
        output += c;
    }
    return output;
}

bstr Stream::read_to_zero(const size_t bytes)
{
    bstr output = read(bytes);
    for (auto i : algo::range(output.size()))
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
    return algo::from_little_endian<u16>(ret);
}

u32 Stream::read_u32_le()
{
    u32 ret = 0;
    read_impl(&ret, 4);
    return algo::from_little_endian<u32>(ret);
}

u64 Stream::read_u64_le()
{
    u64 ret = 0;
    read_impl(&ret, 8);
    return algo::from_little_endian<u64>(ret);
}

f32 Stream::read_f32_le()
{
    return read_any_primitive<const f32, algo::Endianness::LittleEndian>(*this);
}

f64 Stream::read_f64_le()
{
    return read_any_primitive<const f64, algo::Endianness::LittleEndian>(*this);
}

u16 Stream::read_u16_be()
{
    u16 ret = 0;
    read_impl(&ret, 2);
    return algo::from_big_endian<u16>(ret);
}

u32 Stream::read_u32_be()
{
    u32 ret = 0;
    read_impl(&ret, 4);
    return algo::from_big_endian<u32>(ret);
}

u64 Stream::read_u64_be()
{
    u64 ret = 0;
    read_impl(&ret, 8);
    return algo::from_big_endian<u64>(ret);
}

f32 Stream::read_f32_be()
{
    return read_any_primitive<const f32, algo::Endianness::BigEndian>(*this);
}

f64 Stream::read_f64_be()
{
    return read_any_primitive<const f64, algo::Endianness::BigEndian>(*this);
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
    value = algo::to_little_endian<u16>(value);
    write_impl(&value, 2);
    return *this;
}

Stream &Stream::write_u32_le(u32 value)
{
    value = algo::to_little_endian<u32>(value);
    write_impl(&value, 4);
    return *this;
}

Stream &Stream::write_u64_le(u64 value)
{
    value = algo::to_little_endian<u64>(value);
    write_impl(&value, 8);
    return *this;
}

Stream &Stream::write_u16_be(u16 value)
{
    value = algo::to_big_endian<u16>(value);
    write_impl(&value, 2);
    return *this;
}

Stream &Stream::write_u32_be(u32 value)
{
    value = algo::to_big_endian<u32>(value);
    write_impl(&value, 4);
    return *this;
}

Stream &Stream::write_u64_be(u64 value)
{
    value = algo::to_big_endian<u64>(value);
    write_impl(&value, 8);
    return *this;
}
