#pragma once

#include "io/stream.h"

namespace au {
namespace util {
namespace pack {

    enum class ZlibKind : u8
    {
        RawDeflate = 0,
        PlainZlib  = 1, // == RawDeflate + 6 bytes of header+footer
        Gzip       = 2, // == PlainZlib + variable header data
    };

    bstr zlib_inflate(
        io::Stream &io,
        const ZlibKind kind = ZlibKind::PlainZlib);

    bstr zlib_inflate(
        const bstr &input,
        const ZlibKind kind = ZlibKind::PlainZlib);

} } }
