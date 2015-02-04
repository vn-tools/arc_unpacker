#include <stdbool.h>
#include "assert_ex.h"
#include <stdint.h>
#include "endian.h"

int main(void)
{
    const char *x = "\x12\x34\x56\x78";
    bool big_endian = *(uint32_t*)x == 0x12345678;
    bool little_endian = !big_endian;
    if (little_endian)
    {
        assert_equali(le16toh(*(uint16_t*)x), 0x3412);
        assert_equali(be16toh(*(uint16_t*)x), 0x1234);
        assert_equali(htole16(*(uint16_t*)x), 0x3412);
        assert_equali(htobe16(*(uint16_t*)x), 0x1234);
        assert_equali(le32toh(*(uint32_t*)x), 0x78563412);
        assert_equali(be32toh(*(uint32_t*)x), 0x12345678);
        assert_equali(htole32(*(uint32_t*)x), 0x78563412);
        assert_equali(htobe32(*(uint32_t*)x), 0x12345678);
    }
    else
    {
        assert_equali(be16toh(*(uint16_t*)x), 0x3412);
        assert_equali(le16toh(*(uint16_t*)x), 0x1234);
        assert_equali(htobe16(*(uint16_t*)x), 0x3412);
        assert_equali(htole16(*(uint16_t*)x), 0x1234);
        assert_equali(be32toh(*(uint32_t*)x), 0x78563412);
        assert_equali(le32toh(*(uint32_t*)x), 0x12345678);
        assert_equali(htobe32(*(uint32_t*)x), 0x78563412);
        assert_equali(htole32(*(uint32_t*)x), 0x12345678);
    }
    return 0;
}
