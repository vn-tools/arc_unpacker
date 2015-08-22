#include "fmt/kirikiri/tlg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::kirikiri;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    TlgConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding TLG5 works")
{
    do_test(
        "tests/fmt/kirikiri/files/tlg5/14凛ペンダント.tlg",
        "tests/fmt/kirikiri/files/tlg5/14凛ペンダント-out.png");
}

TEST_CASE("Decoding TLG6 works")
{
    do_test(
        "tests/fmt/kirikiri/files/tlg6/tlg6.tlg",
        "tests/fmt/kirikiri/files/tlg6/tlg6-out.png");
}

TEST_CASE("Decoding TLG0 works")
{
    do_test(
        "tests/fmt/kirikiri/files/tlg0/bg08d.tlg",
        "tests/fmt/kirikiri/files/tlg0/bg08d-out.png");
}
