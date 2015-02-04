#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "endian.h"

int main(void)
{
    const char *x = "\x12\x34\x56\x78";
    bool big_endian = *(uint32_t*)x == 0x12345678;
    bool little_endian = !big_endian;
    if (little_endian)
    {
        assert(le16toh(*(uint16_t*)x) == 0x3412);
        assert(be16toh(*(uint16_t*)x) == 0x1234);
        assert(htole16(*(uint16_t*)x) == 0x3412);
        assert(htobe16(*(uint16_t*)x) == 0x1234);
        assert(le32toh(*(uint32_t*)x) == 0x78563412);
        assert(be32toh(*(uint32_t*)x) == 0x12345678);
        assert(htole32(*(uint32_t*)x) == 0x78563412);
        assert(htobe32(*(uint32_t*)x) == 0x12345678);
    }
    else
    {
        assert(be16toh(*(uint16_t*)x) == 0x3412);
        assert(le16toh(*(uint16_t*)x) == 0x1234);
        assert(htobe16(*(uint16_t*)x) == 0x3412);
        assert(htole16(*(uint16_t*)x) == 0x1234);
        assert(be32toh(*(uint32_t*)x) == 0x78563412);
        assert(le32toh(*(uint32_t*)x) == 0x12345678);
        assert(htobe32(*(uint32_t*)x) == 0x78563412);
        assert(htole32(*(uint32_t*)x) == 0x12345678);
    }
    return 0;
}
