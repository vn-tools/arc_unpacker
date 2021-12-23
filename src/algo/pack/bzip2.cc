// Copyright (C) 2018 by notkyon, 2016 rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "algo/pack/bzip2.h"
#include <cstring>
#include <memory>
#include <bzlib.h>
#include "algo/format.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::algo::pack;

static const int buffer_size = 8192;

constexpr uint64_t combine64( uint32_t lo32, uint32_t hi32 ) {
	return uint64_t(hi32)<<32 | uint64_t(lo32);
}
inline uint64_t total_out( const bz_stream &s ) {
	return combine64( s.total_out_lo32, s.total_out_hi32 );
}
inline uint64_t total_in( const bz_stream &s ) {
	return combine64( s.total_in_lo32, s.total_in_hi32 );
}

static bstr process_stream(
    io::BaseByteStream &input_stream,
    const std::function<int(bz_stream &s)> &init_func,
    const std::function<int(bz_stream &s)> &process_func,
    const std::function<int(bz_stream &s)> &end_func,
    const std::string &error_message)
{
    bz_stream s;
    std::memset(&s, 0, sizeof(s));
    if (init_func(s) != BZ_OK)
        throw std::logic_error("Failed to initialize zlib stream");

    bstr output, input_chunk, output_chunk(buffer_size);
    size_t written = 0;
    int ret;
    const auto initial_pos = input_stream.pos();
    do
    {
        if (s.avail_in == 0)
        {
            input_chunk = input_stream.read(
                std::min<size_t>(input_stream.left(), buffer_size));
            s.next_in = const_cast<char*>(input_chunk.get<const char>());
            s.avail_in = input_chunk.size();
        }

        s.next_out = output_chunk.get<char>();
        s.avail_out = output_chunk.size();

        ret = process_func(s);
        output += output_chunk.substr(0, total_out(s) - written);
        written = total_out(s);
        if (ret == BZ_PARAM_ERROR) // zlib equivalent: Z_BUF_ERROR
        {
            input_chunk += input_stream.read(
                std::min<size_t>(input_stream.left(), buffer_size));
            s.next_in = const_cast<char*>(input_chunk.get<const char>());
            s.avail_in = input_chunk.size();
        }
    }
    while (ret == BZ_OK);

    input_stream.seek(initial_pos + total_in(s));
    const auto pos = s.next_in - input_chunk.get<const char>();
    end_func(s);
    if (ret != BZ_STREAM_END)
    {
        throw err::CorruptDataError(algo::format(
            "%s (%s near %x)",
            error_message.c_str(),
            "unknown error",
            pos));
    }
    return output;
}

bstr algo::pack::bz2_inflate(
    io::BaseByteStream &input_stream)
{
    return process_stream(
        input_stream,
        [](bz_stream &s)
        {
            return BZ2_bzDecompressInit(&s, /*verbosity=*/0, /*small=*/0);
        },
        [](bz_stream &s)
        {
            return BZ2_bzDecompress(&s);
        },
        [](bz_stream &s)
        {
            return BZ2_bzDecompressEnd(&s);
        },
        "Failed to inflate bzip2 stream");
}

bstr algo::pack::bz2_inflate(const bstr &input)
{
    io::MemoryByteStream input_stream(input);
    return ::bz2_inflate(input_stream);
}

bstr algo::pack::bz2_deflate(
    const bstr &input,
    const CompressionLevel compression_level)
{
    io::MemoryByteStream input_stream(input);
    return process_stream(
        input_stream,
        [compression_level](bz_stream &s)
        {
			/* http://bzip.org/1.0.5/bzip2-manual-1.0.5.html#bzcompress-init */

			const std::array<int,4> levels { 9, 6, 1, 0 };

            return BZ2_bzCompressInit(
                &s,
                levels.at(static_cast<int>(compression_level)),
                /*verbosity=*/0,
				/*workFactory=*/200);
        },
        [&input_stream](bz_stream &s)
        {
            return BZ2_bzCompress(&s, input_stream.left() ? BZ_RUN : BZ_FINISH);
        },
        [](bz_stream &s)
        {
            return BZ2_bzCompressEnd(&s);
        },
        "Failed to deflate stream");
}
