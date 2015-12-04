#include "io/file.h"
#include "test_support/catch.hh"

using namespace au;

static void test_guessing_extension(
    const bstr &magic, const std::string &expected_extension)
{
    io::File file("test", magic);
    REQUIRE(!file.path.has_extension(expected_extension));
    file.guess_extension();
    INFO("Extension guess test failed for ." << expected_extension);
    REQUIRE(file.path.has_extension(expected_extension));
}

TEST_CASE("File", "[io]")
{
    SECTION("Virtual files")
    {
        SECTION("Via properties")
        {
            io::File file;
            REQUIRE(file.path == "");
            REQUIRE(file.stream.size() == 0);
        }
        SECTION("Via constructor")
        {
            io::File file("test.path", "test.content"_b);
            REQUIRE(file.path == "test.path");
            REQUIRE(file.stream.read_to_eof() == "test.content"_b);
        }
    }

    SECTION("Physical files")
    {
        io::File file("tests/io/file_test.cc", io::FileMode::Read);
        REQUIRE(file.path == "tests/io/file_test.cc");
        REQUIRE(file.stream.read_to_eof().find("TEST_CASE"_b) != bstr::npos);
    }

    SECTION("Guessing extension")
    {
        test_guessing_extension("abmp"_b, "b");
        test_guessing_extension("IMOAVI"_b, "imoavi");
        test_guessing_extension("\x89PNG"_b, "png");
        test_guessing_extension("BM"_b, "bmp");
        test_guessing_extension("RIFF"_b, "wav");
        test_guessing_extension("OggS"_b, "ogg");
        test_guessing_extension("\xFF\xD8\xFF"_b, "jpeg");
    }
}
