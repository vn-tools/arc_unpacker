#include "algo/crypt/blowfish.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("Blowfish decoding", "[algo][crypt]")
{
    SECTION("Aligned to block size")
    {
        static const bstr test_string = "12345678"_b;
        static const bstr test_key = "test_key"_b;
        const Blowfish bf(test_key);
        tests::compare_binary(
            bf.decrypt(bf.encrypt(test_string)),
            test_string);
    }

    SECTION("Not aligned to block size")
    {
        static const bstr test_key = "test_key"_b;
        const Blowfish bf(test_key);
        tests::compare_binary(
            bf.decrypt(bf.encrypt("1234"_b)),
            "1234\x00\x00\x00\x00"_b);
    }
}
