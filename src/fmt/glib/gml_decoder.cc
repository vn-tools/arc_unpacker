#include "fmt/glib/gml_decoder.h"

using namespace au;
using namespace au::fmt::glib;

// modified LZSS
bstr GmlDecoder::decode(const bstr &source, size_t target_size)
{
    bstr target(target_size);
    u8 *target_ptr = target.get<u8>();
    u8 *target_end = target_ptr + target.size();
    const u8 *source_ptr = source.get<const u8>();
    const u8 *source_end = source_ptr + source.size();

    int unk1 = 0;
    while (target_ptr < target_end)
    {
        int carry = unk1 & 1;
        unk1 >>= 1;
        if (!unk1)
        {
            unk1 = source_ptr < source_end ? *source_ptr++ : 0;
            carry = unk1 & 1;
            unk1 = (unk1 >> 1) | 0x80;
        }

        if (carry)
        {
            *target_ptr++ = source_ptr < source_end ? *source_ptr++ : 0;
            continue;
        }

        u8 tmp1 = source_ptr < source_end ? *source_ptr++ : 0;
        u8 tmp2 = source_ptr < source_end ? *source_ptr++ : 0;
        size_t size = ((~tmp2) & 0xF) + 3;
        s32 look_behind = (tmp1 | ((tmp2 << 4) & 0xF00)) + 18;
        look_behind -= target_ptr - target.get<u8>();
        look_behind |= 0xFFFFF000;

        while (target_ptr - target.get<u8>() + look_behind < 0
            && size
            && target_ptr < target_end)
        {
            --size;
            *target_ptr++ = 0;
        }

        while (size && target_ptr < target_end)
        {
            --size;
            u8 tmp = target_ptr[look_behind];
            *target_ptr++ = tmp;
        }
    }

    return target;
}
