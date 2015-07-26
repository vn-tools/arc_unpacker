#ifndef AU_UTIL_ENDIAN_H
#define AU_UTIL_ENDIAN_H
#include <limits.h>
#include <stdint.h>

#ifndef be16toh
    #if CHAR_BIT != 8
        #error "unsupported char size"
    #endif

    #ifndef __BYTE_ORDER__
        #if defined(_WIN32)
            #include <winsock2.h>

            #if BYTE_ORDER == LITTLE_ENDIAN

                #define htobe16(x) htons(x)
                #define htole16(x) (x)
                #define be16toh(x) ntohs(x)
                #define le16toh(x) (x)
                #define htobe32(x) htonl(x)
                #define htole32(x) (x)
                #define be32toh(x) ntohl(x)
                #define le32toh(x) (x)
                #define htobe64(x) htonll(x)
                #define htole64(x) (x)
                #define be64toh(x) ntohll(x)
                #define le64toh(x) (x)

            #elif BYTE_ORDER == BIG_ENDIAN

                #define htobe16(x) (x)
                #define htole16(x) __builtin_bswap16(x)
                #define be16toh(x) (x)
                #define le16toh(x) __builtin_bswap16(x)
                #define htobe32(x) (x)
                #define htole32(x) __builtin_bswap32(x)
                #define be32toh(x) (x)
                #define le32toh(x) __builtin_bswap32(x)
                #define htobe64(x) (x)
                #define htole64(x) __builtin_bswap64(x)
                #define be64toh(x) (x)
                #define le64toh(x) __builtin_bswap64(x)

            #else
                #error byte order not supported
            #endif

        #else
            #error "byte order not defined"
        #endif
    #else
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
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
        #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
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
        #else
            #error "unknown byte order"
        #endif
    #endif
#endif

#endif
