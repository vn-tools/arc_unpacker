#include "test_support/catch.hh"
#include "util/crypt/blowfish.h"
#include "types.h"

using namespace au;
using namespace au::util::crypt;

TEST_CASE("blowfish encryption aligned to block size works", "[util][crypt]")
{
    static const bstr test_string = "12345678"_b;
    static const bstr test_key = "test_key"_b;
    Blowfish bf(test_key);
    REQUIRE(bf.decrypt(bf.encrypt(test_string)) == test_string);
}

TEST_CASE(
    "blowfish encryption not aligned to block size works", "[util][crypt]")
{
    static const bstr test_key = "test_key"_b;
    Blowfish bf(test_key);
    REQUIRE(bf.decrypt(bf.encrypt("1234"_b)) == "1234\x00\x00\x00\x00"_b);
}
