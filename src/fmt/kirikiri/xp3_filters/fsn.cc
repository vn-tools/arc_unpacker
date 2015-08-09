#include <memory>
#include "fmt/kirikiri/xp3_filters/fsn.h"
#include "util/range.h"

using namespace au::fmt::kirikiri::xp3_filters;

void Fsn::decode(File &file, u32) const
{
    size_t size = file.io.size();
    file.io.seek(0);

    auto data = file.io.read(size);
    for (auto i : util::range(size))
        data[i] ^= 0x36;

    if (size > 0x2EA29)
        data[0x2EA29] ^= 3;

    if (size > 0x13)
        data[0x13] ^= 1;

    file.io.seek(0);
    file.io.write(data);
}
