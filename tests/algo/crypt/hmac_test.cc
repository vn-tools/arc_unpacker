#include "algo/crypt/hmac.h"
#include "algo/str.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::algo::crypt;

static void compare_hashes(const bstr &actual, const bstr &expected)
{
    INFO(algo::hex(actual.str()) << " != " << algo::hex(expected.str()));
    REQUIRE(actual == expected);
}

TEST_CASE("HMAC", "[algo][crypt]")
{
    compare_hashes(
        algo::crypt::hmac("test"_b, "key"_b, algo::crypt::HmacKind::Sha512),
        "\x28\x7A\x0F\xB8\x9A\x7F\xBD\xFA\x5B\x55\x38\x63\x69\x18\xE5\x37"
        "\xA5\xB8\x30\x65\xE4\xFF\x33\x12\x68\xB7\xAA\xA1\x15\xDD\xE0\x47"
        "\xA9\xB0\xF4\xFB\x5B\x82\x86\x08\xFC\x0B\x63\x27\xF1\x00\x55\xF7"
        "\x63\x7B\x05\x8E\x9E\x0D\xBB\x9E\x69\x89\x01\xA3\xE6\xDD\x46\x1C"_b);
}
