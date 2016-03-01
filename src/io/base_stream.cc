#include "io/base_stream.h"
#include <memory>
#include "err.h"

using namespace au;
using namespace au::io;

BaseStream::~BaseStream() {}

uoff_t BaseStream::left() const
{
    return size() - pos();
}

io::BaseStream &BaseStream::skip(const soff_t offset)
{
    return seek(pos() + offset);
}

io::BaseStream &BaseStream::peek(
    const uoff_t offset, const std::function<void()> &func)
{
    const auto old_pos = pos();
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
