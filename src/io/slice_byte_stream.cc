#include "io/slice_byte_stream.h"
#include <cstring>

using namespace au;
using namespace au::io;

SliceByteStream::SliceByteStream(
    io::BaseByteStream &parent_stream, const uoff_t slice_offset)
        : SliceByteStream(
            parent_stream, slice_offset, parent_stream.size() - slice_offset)
{
}

SliceByteStream::SliceByteStream(
    io::BaseByteStream &parent_stream,
    const uoff_t slice_offset,
    const uoff_t slice_size) :
        parent_stream(parent_stream.clone()),
        slice_offset(slice_offset),
        slice_size(slice_size)
{
    if (slice_size > parent_stream.size() - slice_offset)
        throw err::BadDataSizeError();
}

SliceByteStream::~SliceByteStream()
{
}

void SliceByteStream::seek_impl(const uoff_t offset)
{
    parent_stream->seek(slice_offset + offset);
}

void SliceByteStream::read_impl(void *destination, const size_t size)
{
    const auto chunk = parent_stream->read(size);
    std::memcpy(destination, chunk.get<u8>(), size);
}

void SliceByteStream::write_impl(const void *source, const size_t size)
{
    throw err::NotSupportedError("Not implemented");
}

uoff_t SliceByteStream::pos() const
{
    return parent_stream->pos() - slice_offset;
}

uoff_t SliceByteStream::size() const
{
    return slice_size;
}

void SliceByteStream::resize_impl(const uoff_t new_size)
{
    throw err::NotSupportedError("Not implemented");
}

std::unique_ptr<io::BaseByteStream> SliceByteStream::clone() const
{
    auto ret = std::make_unique<SliceByteStream>(
        *parent_stream, slice_offset, slice_size);
    ret->seek(pos());
    return std::move(ret);
}
