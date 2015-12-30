#include "algo/pack/zlib.h"
#include <memory>
#include <zlib.h>
#include "algo/format.h"
#include "err.h"

using namespace au;
using namespace au::algo::pack;

static const int buffer_size = 8192;

static std::unique_ptr<z_stream> create_stream(const ZlibKind kind)
{
    auto s = std::make_unique<z_stream>();
    s->zalloc = nullptr;
    s->zfree = nullptr;
    s->opaque = nullptr;
    s->total_out = 0;
    s->avail_in = 0;

    int window_bits
        = kind == ZlibKind::RawDeflate ? -MAX_WBITS
        : kind == ZlibKind::PlainZlib ? MAX_WBITS
        : kind == ZlibKind::Gzip ? MAX_WBITS | 16
        : 0;
    if (!window_bits)
        throw std::logic_error("Bad zlib kind");
    if (inflateInit2(s.get(), window_bits) != Z_OK)
        throw std::logic_error("Failed to initialize zlib stream");
    return s;
}

static void finalize_stream(std::unique_ptr<z_stream> s, int ret, int pos)
{
    inflateEnd(s.get());
    if (ret != Z_STREAM_END)
    {
        throw err::CorruptDataError(algo::format(
            "Failed to inflate zlib stream (%s near %x)",
            s->msg ? s->msg : "unknown error",
            pos));
    }
}

bstr algo::pack::zlib_inflate(const bstr &input, const ZlibKind kind)
{
    size_t written = 0;
    bstr output;
    output.reserve(input.size() * 3);
    auto s = create_stream(kind);
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
    const auto pos = s->next_in - input.get<const Bytef>();
    finalize_stream(std::move(s), ret, pos);
    return output;
}

bstr algo::pack::zlib_inflate(io::Stream &stream, const ZlibKind kind)
{
    bstr input, output;
    size_t written = 0;
    auto s = create_stream(kind);
    int ret;
    const auto initial_pos = stream.tell();
    do
    {
        if (s->avail_in == 0)
        {
            input = stream.tell() + buffer_size > stream.size()
                ? stream.read_to_eof()
                : stream.read(buffer_size);
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
            input += stream.tell() + buffer_size > stream.size()
                ? stream.read_to_eof()
                : stream.read(buffer_size);
            s->next_in = const_cast<Bytef*>(input.get<const Bytef>());
            s->avail_in = input.size();
        }
    }
    while (ret == Z_OK);
    stream.seek(initial_pos + s->total_in);
    finalize_stream(std::move(s), ret, stream.tell());
    return output;
}
