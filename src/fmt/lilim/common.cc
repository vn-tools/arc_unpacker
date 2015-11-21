#include "fmt/lilim/common.h"
#include "io/memory_stream.h"
#include "util/pack/lzss.h"

using namespace au;

bstr fmt::lilim::sysd_decompress(const bstr &input)
{
    io::MemoryStream input_stream(input);
    const bool compressed = input_stream.read_u8();
    const auto size_comp = input_stream.read_u32_le();
    const auto size_orig = input_stream.read_u32_le();
    auto data = input_stream.read(size_comp - 9);
    if (compressed)
        data = util::pack::lzss_decompress_bytewise(data, size_orig);
    return data;
}
