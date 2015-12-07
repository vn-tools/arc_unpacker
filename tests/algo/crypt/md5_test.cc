#include "algo/crypt/md5.h"
#include "algo/str.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::algo::crypt;

static void compare_hashes(const bstr &expected, const bstr &actual)
{
    INFO(algo::hex(expected.str()) << " != " << algo::hex(actual.str()));
    REQUIRE(expected == actual);
}

TEST_CASE("MD5", "[algo][crypt]")
{
    SECTION("Plain MD5")
    {
        compare_hashes(
            algo::crypt::md5("test"_b),
            "\x09\x8F\x6B\xCD\x46\x21\xD3\x73"
            "\xCA\xDE\x4E\x83\x26\x27\xB4\xF6"_b);
    }

    SECTION("Custom initialization vector")
    {
        compare_hashes(
            algo::crypt::md5("test"_b, {0, 0, 0, 0}),
            "\x7E\x8E\xFD\x2F\x05\x58\x82\x92"
            "\x58\xC8\x1F\xC9\x59\x81\xCF\xFF"_b);
    }
}
