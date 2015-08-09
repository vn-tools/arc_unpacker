#include <iostream>
#include "test_support/catch.hpp"
#include "util/crypt/blowfish.h"
#include "types.h"

using namespace au;
using namespace au::util::crypt;

TEST_CASE("blowfish encryption aligned to block size works")
{
    static const std::string test_string = "12345678"_s;
    static const std::string test_key = "test_key"_s;
    Blowfish bf(test_key);
    REQUIRE(bf.decrypt(bf.encrypt(test_string)) == test_string);
}

TEST_CASE("blowfish encryption not aligned to block size works")
{
    static const std::string test_key = "test_key"_s;
    Blowfish bf(test_key);
    REQUIRE(bf.decrypt(bf.encrypt("1234"_s)) == "1234\x00\x00\x00\x00"_s);
}
