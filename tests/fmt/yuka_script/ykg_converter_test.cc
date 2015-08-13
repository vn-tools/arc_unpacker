#include "fmt/yuka_script/ykg_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au::fmt::yuka_script;

TEST_CASE("Decoding YKG images works")
{
    YkgConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/yuka_script/files/reimu.ykg",
        "tests/fmt/yuka_script/files/reimu-out.png");
}
