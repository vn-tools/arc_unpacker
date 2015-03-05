#include <memory>
#include <stdexcept>
#include <zlib.h>
#include "util/zlib.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

std::string zlib_inflate(const std::string &input)
{
    std::string output;
    size_t written = 0;
    output.reserve(input.size() * ((input.size() < SIZE_MAX / 10) ? 3 : 1));

    z_stream stream;
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    stream.avail_in = input.size();
    stream.zalloc = (alloc_func)nullptr;
    stream.zfree = (free_func)nullptr;
    stream.opaque = (voidpf)nullptr;
    stream.total_out = 0;

    if (inflateInit(&stream) != Z_OK)
        throw std::runtime_error("Failed to initialize zlib stream");

    int ret;
    do
    {
        const size_t buffer_size = 8192;
        std::unique_ptr<char> output_buffer(new char[buffer_size]);
        stream.next_out = reinterpret_cast<Bytef*>(output_buffer.get());
        stream.avail_out = buffer_size;
        ret = inflate(&stream, 0);
        if (output.size() < stream.total_out)
            output.append(output_buffer.get(), stream.total_out - written);
        written = stream.total_out;
    }
    while (ret == Z_OK);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END)
    {
        throw std::runtime_error(
            "Failed to inflate zlib stream (" + std::string(stream.msg) + ")");
    }

    return output;
}
