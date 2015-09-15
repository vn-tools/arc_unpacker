#include "fmt/entis/mio_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    MioConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::file_from_path(expected_path);
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Decoding ENTIS's lossy LOT/DCT audio works", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/mio/explosion.mio",
        "tests/fmt/entis/files/mio/explosion-out.wav");
}

TEST_CASE("Decoding ENTIS's lossy LOT/DCT+MSS audio works", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/mio/SE_017.mio",
        "tests/fmt/entis/files/mio/SE_017-out.wav");
}
