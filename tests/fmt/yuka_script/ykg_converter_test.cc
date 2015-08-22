#include "fmt/yuka_script/ykg_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::yuka_script;

TEST_CASE("Decoding YKG images works")
{
    YkgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/yuka_script/files/ykg/reimu.ykg",
        "tests/fmt/yuka_script/files/ykg/reimu-out.png");
}
