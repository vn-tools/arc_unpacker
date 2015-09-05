#include "fmt/kirikiri/xp3_filters/mixed_xor.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kirikiri::xp3_filters;

void MixedXor::decode(File &file, u32 key) const
{
    file.io.seek(0);
    auto data = file.io.read_to_eof();
    for (auto i : util::range(0, data.size(), 2))
        data[i] ^= key;
    for (auto i : util::range(1, data.size(), 2))
        data[i] ^= i;
    file.io.seek(0);
    file.io.write(data);
}
