#include "io/slice_stream.h"
#include <cstring>

using namespace au;
using namespace au::io;

SliceStream::SliceStream(
    io::BaseByteStream &parent_stream, const uoff_t slice_offset)
        : SliceStream(
            parent_stream, slice_offset, parent_stream.size() - slice_offset)
{
}

SliceStream::SliceStream(
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

SliceStream::~SliceStream()
{
}

void SliceStream::seek_impl(const uoff_t offset)
{
    parent_stream->seek(slice_offset + offset);
}

void SliceStream::read_impl(void *destination, const size_t size)
{
    const auto chunk = parent_stream->read(size);
    std::memcpy(destination, chunk.get<u8>(), size);
}

void SliceStream::write_impl(const void *source, const size_t size)
{
    throw err::NotSupportedError("Not implemented");
}

uoff_t SliceStream::pos() const
{
    return parent_stream->pos() - slice_offset;
}

uoff_t SliceStream::size() const
{
    return slice_size;
}

void SliceStream::resize_impl(const uoff_t new_size)
{
    throw err::NotSupportedError("Not implemented");
}

std::unique_ptr<io::BaseByteStream> SliceStream::clone() const
{
    auto ret = std::make_unique<SliceStream>(
        *parent_stream, slice_offset, slice_size);
    ret->seek(pos());
    return std::move(ret);
}
