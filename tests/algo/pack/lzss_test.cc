#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::algo::pack;

static void test_bits(const bstr &input, const bstr &expected)
{
    BitwiseLzssSettings settings;
    settings.position_bits = 12;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 0xFEE;
    const auto actual = lzss_decompress(input, expected.size(), settings);
    INFO("Actual: " + actual.str());
    INFO("Expected: " + expected.str());
    REQUIRE(actual == expected);
}

static void test_bytes(const bstr &input, const bstr &expected)
{
    const auto actual = lzss_decompress(input, expected.size());
    INFO("Actual: " + actual.str());
    INFO("Expected: " + expected.str());
    REQUIRE(actual == expected);
}

static void test_bytes(const bstr &input, size_t size)
{
    test_bytes(input, bstr(size, 'a'));
}

TEST_CASE("LZSS unpacking", "[algo][pack]")
{
    SECTION("Bitwise")
    {
        test_bits("\xBAYnwI\x03\xFB\x90"_b, "test test"_b);
        test_bits(
            "\xB0\xC8\x2D\xB6\x9B\x9D\xCE\xD3\x6F\xB7\x48\x2E\x96\xF9\x05\x9E"
            "\xC3\x69\xB7\x48\x2E\x16\x5B\x93\xFC\x12\x41\x66\xB7\xDC\xA4\x16"
            "\x50\x01\xF8"_b,
            "a mission to gain permission for emission"_b);
    }

    SECTION("Bytewise")
    {
        test_bytes("\x07""123"_b, "123"_b);
        test_bytes("\x07""123\xEE\xF0"_b, "123123"_b);
        test_bytes("\x07""123\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF0"_b,
            "123123123123123123123123123"_b);
        test_bytes("\xFFLife is \x0F""code"_b, "Life is code"_b);

        test_bytes("\x01\x61"_b, 1);
        test_bytes("\x03\x61\x61"_b, 2);
        test_bytes("\x07\x61\x61\x61"_b, 3);
        test_bytes("\x0F\x61\x61\x61\x61"_b, 4);
        test_bytes("\x1F\x61\x61\x61\x61\x61"_b, 5);
        test_bytes("\x07\x61\x61\x61\xEE\xF0"_b, 6);
        test_bytes("\x17\x61\x61\x61\xEE\xF0\x61"_b, 7);
        test_bytes("\x37\x61\x61\x61\xEE\xF0\x61\x61"_b, 8);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF0"_b, 9);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF1"_b, 10);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF2"_b, 11);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3"_b, 12);
        test_bytes("\x27\x61\x61\x61\xEE\xF0\xEE\xF3\x61"_b, 13);
        test_bytes("\x67\x61\x61\x61\xEE\xF0\xEE\xF3\x61\x61"_b, 14);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF0"_b, 15);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF1"_b, 16);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF2"_b, 17);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF3"_b, 18);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF4"_b, 19);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF5"_b, 20);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF6"_b, 21);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF7"_b, 22);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF8"_b, 23);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9"_b, 24);
        test_bytes("\x47\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\x61"_b, 25);
        test_bytes("\xC7\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\x61\x61"_b, 26);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF0"_b, 27);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF1'"_b, 28);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF2'"_b, 29);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF3'"_b, 30);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF4'"_b, 31);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF5'"_b, 32);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF6'"_b, 33);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF7'"_b, 34);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF8'"_b, 35);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xF9'"_b, 36);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xFA'"_b, 37);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xFB'"_b, 38);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xFC'"_b, 39);
        test_bytes("\x07\x61\x61\x61\xEE\xF0\xEE\xF3\xEE\xF9\xEE\xFD'"_b, 40);
    }
}
