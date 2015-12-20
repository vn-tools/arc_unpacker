#include "io/path.h"
#include <cstring>
#include "test_support/catch.h"

using namespace au;

static void compare_paths(
    const io::path &actual_path, const io::path &expected_path)
{
    INFO(actual_path.str() << " != " << expected_path.str());
    REQUIRE(actual_path == expected_path);
}

static void test_concatenating(
    const io::path &path,
    const io::path &other_path,
    const io::path &expected_path)
{
    // copy
    compare_paths(path / other_path, expected_path);

    // in place
    io::path path_copy(path);
    path_copy /= other_path;
    compare_paths(path_copy, expected_path);
}

static void test_changing_extension(
    const io::path &path,
    const std::string new_extension,
    const io::path &expected_path)
{
    io::path path_copy(path);
    path_copy.change_extension(new_extension);
    compare_paths(path_copy, expected_path);
}

static void test_changing_stem(
    const io::path &path,
    const std::string new_stem,
    const io::path &expected_path)
{
    io::path path_copy(path);
    path_copy.change_stem(new_stem);
    compare_paths(path_copy, expected_path);
}

TEST_CASE("Paths", "[io]")
{
    SECTION("Constructor")
    {
        SECTION("Empty")
        {
            io::path p;
            REQUIRE(p.str() == "");
        }

        SECTION("C string")
        {
            io::path p("test");
            REQUIRE(p.str() == "test");
        }

        SECTION("C++ string")
        {
            io::path p(std::string("test"));
            REQUIRE(p.str() == "test");
        }
    }

    SECTION("String accessors")
    {
        SECTION("C string")
        {
            io::path p("test");
            REQUIRE(std::strcmp(p.c_str(), "test") == 0);
        }

        SECTION("C++ string")
        {
            io::path p("test");
            REQUIRE(p.str() == "test");
        }

        SECTION("Wide C++ string")
        {
            io::path p(std::string(u8"testą"));
            REQUIRE(p.wstr() == L"testą");
        }
    }

    SECTION("Retrieving names")
    {
        REQUIRE(io::path("test").name() == "test");
        REQUIRE(io::path("test.dat").name() == "test.dat");
        REQUIRE(io::path("/test").name() == "test");
        REQUIRE(io::path("test/test").name() == "test");
        REQUIRE(io::path("/").name() == "");
        REQUIRE(io::path("test/").name().empty());
        REQUIRE(io::path(".").name().empty());
        REQUIRE(io::path("..").name().empty());
        REQUIRE(io::path("./").name().empty());
        REQUIRE(io::path("../").name().empty());
        REQUIRE(io::path("./.").name().empty());
        REQUIRE(io::path("../..").name().empty());
    }

    SECTION("Retrieving stems")
    {
        REQUIRE(io::path("test").stem() == "test");
        REQUIRE(io::path("test.dat").stem() == "test");
        REQUIRE(io::path("/test").stem() == "test");
        REQUIRE(io::path("test/test").stem() == "test");
        REQUIRE(io::path("test/test.dat").stem() == "test");
        REQUIRE(io::path("/").stem() == "");
        REQUIRE(io::path("test/").stem().empty());
        REQUIRE(io::path(".").stem().empty());
        REQUIRE(io::path("..").stem().empty());
        REQUIRE(io::path("./").stem().empty());
        REQUIRE(io::path("../").stem().empty());
        REQUIRE(io::path("./.").stem().empty());
        REQUIRE(io::path("../..").stem().empty());
    }

    SECTION("Retrieving parent")
    {
        compare_paths(io::path("test").parent(), "");
        compare_paths(io::path("test.dat").parent(), "");
        compare_paths(io::path("/test").parent(), "/");
        compare_paths(io::path("1/test").parent(), "1");
        compare_paths(io::path("1/test.dat").parent(), "1");
        compare_paths(io::path("/1/2/test.dat").parent(), "/1/2");
        compare_paths(io::path("/").parent(), "");
        compare_paths(io::path("test/").parent(), "test");
        compare_paths(io::path(".").parent(), "");
        compare_paths(io::path("..").parent(), "");
        compare_paths(io::path("./").parent(), ".");
        compare_paths(io::path("../").parent(), "..");
        compare_paths(io::path("./.").parent(), ".");
        compare_paths(io::path("../..").parent(), "..");

        compare_paths(io::path("/dir/file.txt").parent().name(), "dir");
        compare_paths(io::path("/dir.ext/file.txt").parent().name(), "dir.ext");
    }

    SECTION("Concatenating")
    {
        test_concatenating("test",          "test", "test/test");
        test_concatenating("test.dat",      "test", "test.dat/test");
        test_concatenating("/test",         "test", "/test/test");
        test_concatenating("1/test",        "test", "1/test/test");
        test_concatenating("1/test.dat",    "test", "1/test.dat/test");
        test_concatenating("/1/2/test.dat", "test", "/1/2/test.dat/test");
        test_concatenating("/",             "test", "/test");
        test_concatenating("test/",         "test", "test/test");
        test_concatenating(".",             "test", "./test");
        test_concatenating("..",            "test", "../test");
        test_concatenating("./",            "test", "./test");
        test_concatenating("../",           "test", "../test");
        test_concatenating("./.",           "test", "././test");
        test_concatenating("../..",         "test", "../../test");
        test_concatenating("../..",         "..", "../../..");
        test_concatenating("../..",         "../", "../../../");
    }

    SECTION("Platform-agnostic slash comparing")
    {
        REQUIRE(io::path("test/test.dat") == io::path("test/test.dat"));
        REQUIRE(io::path("test\\test.dat") == io::path("test\\test.dat"));
        REQUIRE(io::path("C:\\x\\test.dat") == io::path("C:\\x\\test.dat"));
    }

    SECTION("Retrieving extensions")
    {
        REQUIRE(io::path("test").extension().empty());
        REQUIRE(io::path("test.dat").extension() == ".dat");
        REQUIRE(io::path("test/").extension().empty());
        REQUIRE(io::path(".").extension().empty());
        REQUIRE(io::path("..").extension().empty());
        REQUIRE(io::path("./").extension().empty());
        REQUIRE(io::path("../").extension().empty());
        REQUIRE(io::path("./.").extension().empty());
        REQUIRE(io::path("../..").extension().empty());
    }

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

    SECTION("Changing stem")
    {
        test_changing_stem("",              "xyz",  "xyz");
        test_changing_stem(".",             "xyz",  "xyz");
        test_changing_stem("..",            "xyz",  "xyz");
        test_changing_stem("abc",           "xyz",  "xyz");
        test_changing_stem("abc.de",        "xyz",  "xyz.de");
        test_changing_stem("abc.de",        "xyz.", "xyz..de");
        test_changing_stem("abc.",          "xyz",  "xyz");
        test_changing_stem(".abc.",         "xyz",  "xyz");
        test_changing_stem("abc/",          "xyz",  "abc/xyz");
        test_changing_stem("abc/.",         "xyz",  "abc/xyz");
        test_changing_stem("abc/..",        "xyz",  "abc/xyz");
        test_changing_stem("abc/../",       "xyz",  "abc/../xyz");
        test_changing_stem("abc/def",       "xyz",  "abc/xyz");
        test_changing_stem("abc/def.",      "xyz",  "abc/xyz");
        test_changing_stem("abc/.def.",     "xyz",  "abc/xyz");
        test_changing_stem("./abc/def.gh",  "xyz",  "./abc/xyz.gh");
        test_changing_stem("../abc/def.gh", "xyz",  "../abc/xyz.gh");
        test_changing_stem("abc/./def.gh",  "xyz",  "abc/./xyz.gh");
        test_changing_stem("abc/../def.gh", "xyz",  "abc/../xyz.gh");
        test_changing_stem("./abc/def",     "xyz",  "./abc/xyz");
        test_changing_stem("../abc/def",    "xyz",  "../abc/xyz");
        test_changing_stem("abc/./def",     "xyz",  "abc/./xyz");
        test_changing_stem("abc/../def",    "xyz",  "abc/../xyz");
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
        test_changing_extension("abc/",          "xyz",  "abc/");
        test_changing_extension("abc/.",         "xyz",  "abc/.");
        test_changing_extension("abc/..",        "xyz",  "abc/..");
        test_changing_extension("abc/../",       "xyz",  "abc/../");
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
        test_changing_extension("abc/",          "",  "abc/");
        test_changing_extension("abc/.",         "",  "abc/.");
        test_changing_extension("abc/..",        "",  "abc/..");
        test_changing_extension("abc/../",       "",  "abc/../");
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
