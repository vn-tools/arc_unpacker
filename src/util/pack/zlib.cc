#include <memory>
#include <stdexcept>
#include <zlib.h>
#include "util/format.h"
#include "util/pack/zlib.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

bstr au::util::pack::zlib_inflate(const bstr &input)
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
        throw std::runtime_error("Failed to initialize zlib stream");

    int ret;
    do
    {
        const size_t buffer_size = 8192;
        bstr output_chunk;
        output_chunk.resize(buffer_size);
        stream.next_out = output_chunk.get<Bytef>();
        stream.avail_out = buffer_size;
        ret = inflate(&stream, 0);
        if (output.size() < stream.total_out)
            output += output_chunk.substr(0, stream.total_out - written);
        written = stream.total_out;
    }
    while (ret == Z_OK);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END)
    {
        throw std::runtime_error(util::format(
            "Failed to inflate zlib stream (%s)", stream.msg));
    }

    return output;
}
