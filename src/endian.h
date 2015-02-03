#ifndef ORDER32_H
#define ORDER32_H

#include <limits.h>
#include <stdint.h>

#ifndef be16toh
#if CHAR_BIT != 8
    #error "unsupported char size"
#endif

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302ul
};

static const union
{
    unsigned char bytes[4];
    uint32_t value;
} o32_host_order = { { 0, 1, 2, 3 } };

//#define O32_HOST_ORDER (o32_host_order.value)
#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})

#if O32_HOST_ORDER == O32_LITTLE_ENDIAN
    #define be16toh(x) __builtin_bswap16(x)
    #define be32toh(x) __builtin_bswap32(x)
    #define be64toh(x) __builtin_bswap64(x)
    #define htobe16(x) __builtin_bswap16(x)
    #define htobe32(x) __builtin_bswap32(x)
    #define htobe64(x) __builtin_bswap64(x)

    #define le16toh(x) (x)
    #define le32toh(x) (x)
    #define le64toh(x) (x)
    #define htole16(x) (x)
    #define htole32(x) (x)
    #define htole64(x) (x)
#else
    #define be16toh(x) (x)
    #define be32toh(x) (x)
    #define be64toh(x) (x)
    #define htobe16(x) (x)
    #define htobe32(x) (x)
    #define htobe64(x) (x)

    #define le16toh(x) __builtin_bswap16(x)
    #define le32toh(x) __builtin_bswap32(x)
    #define le64toh(x) __builtin_bswap64(x)
    #define htole16(x) __builtin_bswap16(x)
    #define htole32(x) __builtin_bswap32(x)
    #define htole64(x) __builtin_bswap64(x)
#endif
#endif

#endif
