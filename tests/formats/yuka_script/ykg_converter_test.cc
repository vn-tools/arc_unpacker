#include "formats/yuka_script/ykg_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt::yuka_script;

TEST_CASE("Decoding YKG images works")
{
    YkgConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/formats/yuka_script/files/reimu.ykg",
        "tests/formats/yuka_script/files/reimu-out.png");
}
