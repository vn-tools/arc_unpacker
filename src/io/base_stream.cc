#include "io/base_stream.h"
#include <algorithm>
#include <memory>
#include "algo/endian.h"
#include "algo/range.h"

using namespace au;
using namespace au::io;

io::IStream &BaseStream::peek(const size_t offset, std::function<void()> func)
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

bool BaseStream::eof() const
{
    return tell() == size();
}

bstr BaseStream::read_to_zero()
{
    bstr output;
    while (!eof())
    {
        char c = read<u8>();
        if (c == '\0')
            break;
        output += c;
    }
    return output;
}

bstr BaseStream::read_to_zero(const size_t bytes)
{
    bstr output = read(bytes);
    for (auto i : algo::range(output.size()))
        if (!output[i])
            return output.substr(0, i);
    return output;
}

bstr BaseStream::read_to_eof()
{
    return read(size() - tell());
}

bstr BaseStream::read_line()
{
    bstr output;
    char c;
    while (!eof())
    {
        c = read<u8>();
        if (c == '\0' || c == '\n')
            break;
        if (c != '\r')
            output += c;
    }
    return output;
}
