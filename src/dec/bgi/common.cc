#include "dec/bgi/common.h"

using namespace au;

u8 dec::bgi::get_and_update_key(u32 &key)
{
    const u32 tmp1 = 0x4E35 * (key & 0xFFFF);
    const u32 tmp2 = 0x4E35 * (key >> 16);
    const u32 tmp = 0x15A * key + tmp2 + (tmp1 >> 16);
    key = (tmp << 16) + (tmp1 & 0xFFFF) + 1;
    return static_cast<u8>(tmp & 0x7FFF);
}
