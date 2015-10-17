#include "fmt/shiina_rio/warc/decompress.h"
#include "err.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;
using namespace au::fmt::shiina_rio::warc;

bstr warc::decompress_yh1(const bstr &input, const size_t size_orig)
{
    throw err::NotSupportedError("YH1 decompression not implemented");
}

bstr warc::decompress_ypk(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    bstr transient(input);
    if (encrypted)
    {
        u16 key16 = 0x4B4D;
        u32 key32 = (key16 | (key16 << 16)) ^ 0xFFFFFFFF;
        size_t i = 0;
        while (i < transient.size() / 4)
            transient.get<u32>()[i++] ^= key32;
        i *= 4;
        while (i < transient.size())
            transient[i++] ^= key32;
    }
    return util::pack::zlib_inflate(transient);
}

bstr warc::decompress_ylz(const bstr &input, const size_t size_orig)
{
    throw err::NotSupportedError("YLZ decompression not implemented");
}
