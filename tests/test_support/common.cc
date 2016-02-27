#include "test_support/common.h"
#include "algo/str.h"
#include "test_support/catch.h"

using namespace au;

void au::tests::compare_paths(
    const io::path &actual, const io::path &expected)
{
    INFO(actual.str() << " != " << expected.str());
    REQUIRE(actual == expected);
}

void au::tests::compare_binary(const bstr &actual, const bstr &expected)
{
    const auto max_size = 10000;
    auto actual_dump = algo::hex(actual);
    auto expected_dump = algo::hex(expected);
    if (actual_dump.size() > max_size)
        actual_dump = actual_dump.substr(0, max_size) + "(...)";
    if (expected_dump.size() > max_size)
        expected_dump = expected_dump.substr(0, max_size) + "(...)";
    INFO(actual_dump << " != " << expected_dump);
    REQUIRE(actual == expected);
}
