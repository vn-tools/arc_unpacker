#include "test_support/catch.hpp"
#include "util/endian.h"

TEST_CASE("Converting endianness works")
{
    const char *x = "\x12\x34\x56\x78";
    bool big_endian = *reinterpret_cast<const uint32_t*>(x) == 0x12345678;
    if (big_endian)
    {
        REQUIRE(be16toh(*reinterpret_cast<const uint16_t*>(x)) == 0x3412);
        REQUIRE(le16toh(*reinterpret_cast<const uint16_t*>(x)) == 0x1234);
        REQUIRE(htobe16(*reinterpret_cast<const uint16_t*>(x)) == 0x3412);
        REQUIRE(htole16(*reinterpret_cast<const uint16_t*>(x)) == 0x1234);
        REQUIRE(be32toh(*reinterpret_cast<const uint32_t*>(x)) == 0x78563412);
        REQUIRE(le32toh(*reinterpret_cast<const uint32_t*>(x)) == 0x12345678);
        REQUIRE(htobe32(*reinterpret_cast<const uint32_t*>(x)) == 0x78563412);
        REQUIRE(htole32(*reinterpret_cast<const uint32_t*>(x)) == 0x12345678);
    }
    else
    {
        REQUIRE(le16toh(*reinterpret_cast<const uint16_t*>(x)) == 0x3412);
        REQUIRE(be16toh(*reinterpret_cast<const uint16_t*>(x)) == 0x1234);
        REQUIRE(htole16(*reinterpret_cast<const uint16_t*>(x)) == 0x3412);
        REQUIRE(htobe16(*reinterpret_cast<const uint16_t*>(x)) == 0x1234);
        REQUIRE(le32toh(*reinterpret_cast<const uint32_t*>(x)) == 0x78563412);
        REQUIRE(be32toh(*reinterpret_cast<const uint32_t*>(x)) == 0x12345678);
        REQUIRE(htole32(*reinterpret_cast<const uint32_t*>(x)) == 0x78563412);
        REQUIRE(htobe32(*reinterpret_cast<const uint32_t*>(x)) == 0x12345678);
    }
}
