#include "formats/glib/gml_decoder.h"

// modified LZSS
void GmlDecoder::decode(BufferedIO &source_io, BufferedIO &target_io)
{
    char *target = target_io.buffer();
    char *target_ptr = target;
    char *target_guardian = target + target_io.size();

    int unk1 = 0;
    while (target_ptr < target_guardian)
    {
        int carry = unk1 & 1;
        unk1 >>= 1;
        if (!unk1)
        {
            unk1 = source_io.read_u8();
            carry = unk1 & 1;
            unk1 = (unk1 >> 1) | 0x80;
        }

        if (carry)
        {
            *target_ptr ++ = source_io.read_u8();
            continue;
        }

        uint8_t tmp1 = source_io.read_u8();
        uint8_t tmp2 = source_io.read_u8();
        size_t length = ((~tmp2) & 0xf) + 3;
        int32_t look_behind = (tmp1 | ((tmp2 << 4) & 0xf00)) + 18;
        look_behind -= target_ptr - target;
        look_behind |= 0xfffff000;

        while (target_ptr - target + look_behind < 0
            && length
            && target_ptr < target_guardian)
        {
            -- length;
            *target_ptr ++ = 0;
        }

        while (length && target_ptr < target_guardian)
        {
            -- length;
            uint8_t tmp = target_ptr[look_behind];
            *target_ptr ++ = tmp;
        }
    }

    target_io.write(target, target_io.size());
}
