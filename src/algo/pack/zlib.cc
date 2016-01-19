#include "algo/pack/zlib.h"
#include <cstring>
#include <memory>
#include <zlib.h>
#include "algo/format.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::algo::pack;

static const int buffer_size = 8192;

static bstr process_stream(
    io::IStream &input_stream,
    const ZlibKind kind,
    const std::function<int(z_stream &s, const int window_bits)> &init_func,
    const std::function<int(z_stream &s)> &process_func,
    const std::function<int(z_stream &s)> &end_func,
    const std::string &error_message)
{
    const int window_bits
        = kind == ZlibKind::RawDeflate ? -MAX_WBITS
        : kind == ZlibKind::PlainZlib ? MAX_WBITS
        : kind == ZlibKind::Gzip ? MAX_WBITS | 16
        : 0;
    if (!window_bits)
        throw std::logic_error("Bad zlib kind");

    z_stream s;
    std::memset(&s, 0, sizeof(s));
    if (init_func(s, window_bits) != Z_OK)
        throw std::logic_error("Failed to initialize zlib stream");

    bstr output, input_chunk, output_chunk(buffer_size);
    size_t written = 0;
    int ret;
    const auto initial_pos = input_stream.tell();
    do
    {
        if (s.avail_in == 0)
        {
            input_chunk = input_stream.read(
                std::min<size_t>(input_stream.left(), buffer_size));
            s.next_in = const_cast<Bytef*>(input_chunk.get<const Bytef>());
            s.avail_in = input_chunk.size();
        }

        s.next_out = output_chunk.get<Bytef>();
        s.avail_out = output_chunk.size();

        ret = process_func(s);
        output += output_chunk.substr(0, s.total_out - written);
        written = s.total_out;
        if (ret == Z_BUF_ERROR)
        {
            input_chunk += input_stream.read(
                std::min<size_t>(input_stream.left(), buffer_size));
            s.next_in = const_cast<Bytef*>(input_chunk.get<const Bytef>());
            s.avail_in = input_chunk.size();
        }
    }
    while (ret == Z_OK);

    input_stream.seek(initial_pos + s.total_in);
    const auto pos = s.next_in - input_chunk.get<const Bytef>();
    end_func(s);
    if (ret != Z_STREAM_END)
    {
        throw err::CorruptDataError(algo::format(
            "%s (%s near %x)",
            error_message.c_str(),
            s.msg ? s.msg : "unknown error",
            pos));
    }
    return output;
}

bstr algo::pack::zlib_inflate(io::IStream &input_stream, const ZlibKind kind)
{
    return process_stream(
        input_stream,
        kind,
        [](z_stream &s, const int window_bits)
        {
            return inflateInit2(&s, window_bits);
        },
        [](z_stream &s)
        {
            return inflate(&s, Z_NO_FLUSH);
        },
        [](z_stream &s)
        {
            return inflateEnd(&s);
        },
        "Failed to inflate zlib stream");
}

bstr algo::pack::zlib_inflate(const bstr &input, const ZlibKind kind)
{
    io::MemoryStream input_stream(input);
    return ::zlib_inflate(input_stream, kind);
}

bstr algo::pack::zlib_deflate(const bstr &input, const ZlibKind kind)
{
    io::MemoryStream input_stream(input);
    return process_stream(
        input_stream,
        kind,
        [](z_stream &s, const int window_bits)
        {
            return deflateInit2(
                &s, 9, Z_DEFLATED, window_bits, 9, Z_DEFAULT_STRATEGY);
        },
        [&input_stream](z_stream &s)
        {
            return deflate(&s, input_stream.eof() ? Z_FINISH : Z_NO_FLUSH);
        },
        [](z_stream &s)
        {
            return deflateEnd(&s);
        },
        "Failed to deflate stream");
}
