#include "file.h"
#include "test_support/catch.hh"

using namespace au;

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

TEST_CASE("Empty file creation", "[core][file]")
{
    File file;
    REQUIRE(file.name == "");
}

TEST_CASE("Setting File's name", "[core][file]")
{
    File file;
    file.name = "abc";
    REQUIRE(file.name == "abc");
}

TEST_CASE("Checking File's extension", "[core][file]")
{
    File extensionless_file;
    extensionless_file.name = "test";
    REQUIRE(!extensionless_file.has_extension());
    REQUIRE(!extensionless_file.has_extension("test"));
    REQUIRE(!extensionless_file.has_extension("dat"));
    File dat_file;
    dat_file.name = "test.dat";
    REQUIRE(dat_file.has_extension());
    REQUIRE(!dat_file.has_extension("test"));
    REQUIRE(dat_file.has_extension("dat"));
}

TEST_CASE("Changing File's extension", "[core][file]")
{
    test_changing_extension("",              "xyz",  "");
    test_changing_extension(".",             "xyz",  ".");
    test_changing_extension("..",            "xyz",  "..");
    test_changing_extension("abc",           "xyz",  "abc.xyz");
    test_changing_extension("abc.de",        "xyz",  "abc.xyz");
    test_changing_extension("abc.de",        ".xyz", "abc.xyz");
    test_changing_extension("abc.",          "xyz",  "abc.xyz");
    test_changing_extension(".abc.",         "xyz",  ".abc.xyz");
    test_changing_extension("abc",           "xyz",  "abc.xyz");
    test_changing_extension("abc/",          "xyz",  "abc/");
    test_changing_extension("abc/.",         "xyz",  "abc/.");
    test_changing_extension("abc/..",        "xyz",  "abc/..");
    test_changing_extension("abc/def",       "xyz",  "abc/def.xyz");
    test_changing_extension("abc/def.",      "xyz",  "abc/def.xyz");
    test_changing_extension("abc/.def.",     "xyz",  "abc/.def.xyz");
    test_changing_extension("./abc/def.gh",  "xyz",  "./abc/def.xyz");
    test_changing_extension("../abc/def.gh", "xyz",  "../abc/def.xyz");
    test_changing_extension("abc/./def.gh",  "xyz",  "abc/./def.xyz");
    test_changing_extension("abc/../def.gh", "xyz",  "abc/../def.xyz");
    test_changing_extension("./abc/def",     "xyz",  "./abc/def.xyz");
    test_changing_extension("../abc/def",    "xyz",  "../abc/def.xyz");
    test_changing_extension("abc/./def",     "xyz",  "abc/./def.xyz");
    test_changing_extension("abc/../def",    "xyz",  "abc/../def.xyz");
}

TEST_CASE("Stripping File's extension", "[core][file]")
{
    test_changing_extension("",              "",  "");
    test_changing_extension(".",             "",  ".");
    test_changing_extension("..",            "",  "..");
    test_changing_extension("abc",           "",  "abc");
    test_changing_extension("abc.de",        "",  "abc");
    test_changing_extension("abc.de",        ".", "abc");
    test_changing_extension("abc.",          "",  "abc");
    test_changing_extension(".abc.",         "",  ".abc");
    test_changing_extension("abc",           "",  "abc");
    test_changing_extension("abc/",          "",  "abc/");
    test_changing_extension("abc/.",         "",  "abc/.");
    test_changing_extension("abc/..",        "",  "abc/..");
    test_changing_extension("abc/def",       "",  "abc/def");
    test_changing_extension("abc/def.",      "",  "abc/def");
    test_changing_extension("abc/.def.",     "",  "abc/.def");
    test_changing_extension("./abc/def.gh",  "",  "./abc/def");
    test_changing_extension("../abc/def.gh", "",  "../abc/def");
    test_changing_extension("abc/./def.gh",  "",  "abc/./def");
    test_changing_extension("abc/../def.gh", "",  "abc/../def");
    test_changing_extension("./abc/def",     "",  "./abc/def");
    test_changing_extension("../abc/def",    "",  "../abc/def");
    test_changing_extension("abc/./def",     "",  "abc/./def");
    test_changing_extension("abc/../def",    "",  "abc/../def");
}
