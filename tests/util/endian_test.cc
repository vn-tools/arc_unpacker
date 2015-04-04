#include "util/endian.h"
#include "test_support/eassert.h"

int main(void)
{
    const char *x = "\x12\x34\x56\x78";
    bool big_endian = *(uint32_t*)x == 0x12345678;
    bool little_endian = !big_endian;
    if (little_endian)
    {
        eassert(le16toh(*(uint16_t*)x) == 0x3412);
        eassert(be16toh(*(uint16_t*)x) == 0x1234);
        eassert(htole16(*(uint16_t*)x) == 0x3412);
        eassert(htobe16(*(uint16_t*)x) == 0x1234);
        eassert(le32toh(*(uint32_t*)x) == 0x78563412);
        eassert(be32toh(*(uint32_t*)x) == 0x12345678);
        eassert(htole32(*(uint32_t*)x) == 0x78563412);
        eassert(htobe32(*(uint32_t*)x) == 0x12345678);
    }
    else
    {
        eassert(be16toh(*(uint16_t*)x) == 0x3412);
        eassert(le16toh(*(uint16_t*)x) == 0x1234);
        eassert(htobe16(*(uint16_t*)x) == 0x3412);
        eassert(htole16(*(uint16_t*)x) == 0x1234);
        eassert(be32toh(*(uint32_t*)x) == 0x78563412);
        eassert(le32toh(*(uint32_t*)x) == 0x12345678);
        eassert(htobe32(*(uint32_t*)x) == 0x78563412);
        eassert(htole32(*(uint32_t*)x) == 0x12345678);
    }
    return 0;
}
