#include "fmt/lilim/common.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"

using namespace au;

bstr fmt::lilim::sysd_decompress(const bstr &input)
{
    io::BufferedIO input_io(input);
    const bool compressed = input_io.read_u8();
    const auto size_comp = input_io.read_u32_le();
    const auto size_orig = input_io.read_u32_le();
    auto data = input_io.read(size_comp - 9);
    if (compressed)
        data = util::pack::lzss_decompress_bytewise(data, size_orig);
    return data;
}
