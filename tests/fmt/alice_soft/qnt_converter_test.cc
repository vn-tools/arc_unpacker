#include "fmt/alice_soft/qnt_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    QntConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Alice Soft QNT opaque images", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/qnt/CG00505.QNT",
        "tests/fmt/alice_soft/files/qnt/CG00505-out.png");
}

TEST_CASE("Alice Soft QNT transparent images with resolution of 2^n", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/qnt/CG64100.QNT",
        "tests/fmt/alice_soft/files/qnt/CG64100-out.png");
}

TEST_CASE(
    "Alice Soft QNT transparent images with arbitrary resolution", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/qnt/CG64214.QNT",
        "tests/fmt/alice_soft/files/qnt/CG64214-out.png");
}
