#include "util/pack/zlib.h"
#include <memory>
#include <zlib.h>
#include "err.h"
#include "util/format.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

using namespace au;

bstr util::pack::zlib_inflate(const bstr &input)
{
    bstr output;
    size_t written = 0;
    output.reserve(input.size() * ((input.size() < SIZE_MAX / 10) ? 3 : 1));

    z_stream stream;
    stream.next_in = const_cast<Bytef*>(input.get<const Bytef>());
    stream.avail_in = input.size();
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.total_out = 0;

    if (inflateInit(&stream) != Z_OK)
        throw std::logic_error("Failed to initialize zlib stream");

    int ret;
    do
    {
        bstr output_chunk(8192);
        stream.next_out = output_chunk.get<Bytef>();
        stream.avail_out = output_chunk.size();
        ret = inflate(&stream, 0);
        if (output.size() < stream.total_out)
            output += output_chunk.substr(0, stream.total_out - written);
        written = stream.total_out;
    }
    while (ret == Z_OK);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END)
    {
        throw err::CorruptDataError(util::format(
            "Failed to inflate zlib stream (%s)",
            stream.msg ? stream.msg : "unknown error"));
    }

    return output;
}
