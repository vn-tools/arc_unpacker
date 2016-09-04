#include "dec/lilim/common.h"
#include "algo/pack/lzss.h"
#include "io/memory_byte_stream.h"

using namespace au;

bstr dec::lilim::sysd_decompress(const bstr &input)
{
    io::MemoryByteStream input_stream(input);
    const auto compressed = input_stream.read<u8>() != 0;
    const auto size_comp = input_stream.read_le<u32>();
    const auto size_orig = input_stream.read_le<u32>();
    auto data = input_stream.read(size_comp - 9);
    if (compressed)
        data = algo::pack::lzss_decompress(data, size_orig);
    return data;
}
