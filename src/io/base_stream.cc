#include "io/base_stream.h"
#include <memory>
#include "err.h"

using namespace au;
using namespace au::io;

BaseStream::~BaseStream() {}

size_t BaseStream::left() const
{
    return size() - tell();
}

io::BaseStream &BaseStream::skip(const int offset)
{
    if (tell() + offset > size())
        throw err::EofError();
    return seek(tell() + offset);
}

io::BaseStream &BaseStream::peek(
    const size_t offset, std::function<void()> func)
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
