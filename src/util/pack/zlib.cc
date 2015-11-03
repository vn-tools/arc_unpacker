#include "util/pack/zlib.h"
#include <memory>
#include <zlib.h>
#include "err.h"
#include "util/format.h"

using namespace au;

static const int buffer_size = 8192;

static std::unique_ptr<z_stream> create_stream()
{
    auto s = std::make_unique<z_stream>();
    s->zalloc = nullptr;
    s->zfree = nullptr;
    s->opaque = nullptr;
    s->total_out = 0;
    s->avail_in = 0;

    if (inflateInit(s.get()) != Z_OK)
        throw std::logic_error("Failed to initialize zlib stream");
    return s;
}

static void finalize_stream(std::unique_ptr<z_stream> s, int ret, int pos)
{
    inflateEnd(s.get());
    if (ret != Z_STREAM_END)
    {
        throw err::CorruptDataError(util::format(
            "Failed to inflate zlib stream (%s near %x)",
            s->msg ? s->msg : "unknown error",
            pos));
    }
}

bstr util::pack::zlib_inflate(const bstr &input)
{
    size_t written = 0;
    bstr output;
    output.reserve(input.size() * 3);
    auto s = create_stream();
    s->next_in = const_cast<Bytef*>(input.get<const Bytef>());
    s->avail_in = input.size();
    int ret;
    do
    {
        bstr output_chunk(buffer_size);
        s->next_out = output_chunk.get<Bytef>();
        s->avail_out = output_chunk.size();

        ret = inflate(s.get(), 0);
        output += output_chunk.substr(0, s->total_out - written);
        written = s->total_out;
    }
    while (ret == Z_OK);
    finalize_stream(std::move(s), ret, s->next_in - input.get<const Bytef>());
    return output;
}

bstr util::pack::zlib_inflate(io::IO &io)
{
    bstr input, output;
    size_t written = 0;
    auto s = create_stream();
    int ret;
    const auto initial_pos = io.tell();
    do
    {
        if (s->avail_in == 0)
        {
            input = io.tell() + buffer_size > io.size()
                ? io.read_to_eof()
                : io.read(buffer_size);
            s->next_in = const_cast<Bytef*>(input.get<const Bytef>());
            s->avail_in = input.size();
        }

        bstr output_chunk(buffer_size);
        s->next_out = output_chunk.get<Bytef>();
        s->avail_out = output_chunk.size();

        ret = inflate(s.get(), 0);
        output += output_chunk.substr(0, s->total_out - written);
        written = s->total_out;
        if (ret == Z_BUF_ERROR)
        {
            input += io.tell() + buffer_size > io.size()
                ? io.read_to_eof()
                : io.read(buffer_size);
            s->next_in = const_cast<Bytef*>(input.get<const Bytef>());
            s->avail_in = input.size();
        }
    }
    while (ret == Z_OK);
    io.seek(initial_pos + s->total_in);
    finalize_stream(std::move(s), ret, io.tell());
    return output;
}
