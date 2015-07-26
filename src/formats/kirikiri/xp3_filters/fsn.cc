#include <memory>
#include "formats/kirikiri/xp3_filters/fsn.h"

using namespace au::fmt::kirikiri::xp3_filters;

void Fsn::decode(File &file, u32) const
{
    size_t size = file.io.size();
    file.io.seek(0);
    std::unique_ptr<char[]> data(new char[size]);
    char *ptr = data.get();
    file.io.read(ptr, size);

    for (size_t i = 0; i < size; i++)
        ptr[i] ^= 0x36;

    if (size > 0x2ea29)
        ptr[0x2ea29] ^= 3;

    if (size > 0x13)
        ptr[0x13] ^= 1;

    file.io.seek(0);
    file.io.write(ptr, size);
}
