#include "file.h"
#include "test_support/catch.hpp"

static void test_changing_extension(
    const std::string name,
    const std::string new_extension,
    const std::string expected_name)
{
    File file;
    file.name = name;
    file.change_extension(new_extension);
    REQUIRE(file.name == expected_name);
}

TEST_CASE("Empty file creation")
{
    File file;
    REQUIRE(file.name == "");
}

TEST_CASE("Setting name works")
{
    File file;
    file.name = "abc";
    REQUIRE(file.name == "abc");
}

TEST_CASE("Changing file extension work")
{
    test_changing_extension("",          "xyz",  "");
    test_changing_extension(".",         "xyz",  ".");
    test_changing_extension("..",        "xyz",  "..");
    test_changing_extension("abc",       "xyz",  "abc.xyz");
    test_changing_extension("abc.de",    "xyz",  "abc.xyz");
    test_changing_extension("abc.de",    ".xyz", "abc.xyz");
    test_changing_extension("abc.",      "xyz",  "abc.xyz");
    test_changing_extension(".abc.",     "xyz",  ".abc.xyz");
    test_changing_extension("abc",       "xyz",  "abc.xyz");
    test_changing_extension("abc/",      "xyz",  "abc/");
    test_changing_extension("abc/.",     "xyz",  "abc/.");
    test_changing_extension("abc/..",    "xyz",  "abc/..");
    test_changing_extension("abc/def",   "xyz",  "abc/def.xyz");
    test_changing_extension("abc/def.",  "xyz",  "abc/def.xyz");
    test_changing_extension("abc/.def.", "xyz",  "abc/.def.xyz");
}

