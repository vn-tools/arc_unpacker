#include "fmt/entis/eri_converter.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    EriConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding ENTIS's lossless Huffman 32-bit images works")
{
    do_test(
        "tests/fmt/entis/files/eri/img_rgba32.eri",
        "tests/fmt/entis/files/eri/img_rgba32-out.png");
}
