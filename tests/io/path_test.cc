#include "io/path.h"
#include "test_support/catch.hh"

using namespace au;

static void test_changing_extension(
    const io::path &path,
    const std::string new_extension,
    const io::path &expected_path)
{
    io::path path_copy(path);
    path_copy.change_extension(new_extension);
    REQUIRE(path_copy == io::path(expected_path));
}

TEST_CASE("Paths", "[io]")
{
    SECTION("Checking extension")
    {
        io::path extensionless_path("test");
        INFO(extensionless_path.extension());
        REQUIRE(!extensionless_path.has_extension());
        REQUIRE(!extensionless_path.has_extension("test"));
        REQUIRE(!extensionless_path.has_extension("dat"));
        io::path path_with_extension("test.dat");
        REQUIRE(path_with_extension.has_extension());
        REQUIRE(!path_with_extension.has_extension("test"));
        REQUIRE(path_with_extension.has_extension("dat"));
    }

    SECTION("Changing extension")
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

    SECTION("Stripping extension")
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
}
