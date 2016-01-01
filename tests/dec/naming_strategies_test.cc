#include "dec/naming_strategies.h"
#include "test_support/catch.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec;

static void do_test(
    const NamingStrategy strategy,
    const io::path &parent_path,
    const io::path &child_path,
    const io::path &expected_path)
{
    const auto actual_path = dec::decorate_path(
        strategy, parent_path, child_path);
    tests::compare_paths(actual_path, expected_path);
}

TEST_CASE("File naming strategies", "[fmt_core]")
{
    SECTION("Root")
    {
        const auto s = NamingStrategy::Root;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "file");
        do_test(s, "test/",      "file", "file");
        do_test(s, "test/nest",  "file", "file");
        do_test(s, "test/nest/", "file", "file");
        do_test(s, "test/nest",  "a/b", "a/b");
    }

    SECTION("Sibling")
    {
        const auto s = NamingStrategy::Sibling;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "file");
        do_test(s, "test/",      "file", "test/file");
        do_test(s, "test/nest",  "file", "test/file");
        do_test(s, "test/nest/", "file", "test/nest/file");
        do_test(s, "test/nest",  "a/b", "test/a/b");
    }

    SECTION("Flat sibling")
    {
        const auto s = NamingStrategy::FlatSibling;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "file");
        do_test(s, "test/",      "file", "test/file");
        do_test(s, "test/nest",  "file", "test/file");
        do_test(s, "test/nest/", "file", "test/nest/file");
        do_test(s, "test/nest",  "a/b", "test/b");
    }

    SECTION("Child")
    {
        const auto s = NamingStrategy::Child;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "test/file");
        do_test(s, "test/",      "file", "test/file");
        do_test(s, "test/nest",  "file", "test/nest/file");
        do_test(s, "test/nest/", "file", "test/nest/file");
        do_test(s, "test/nest",  "a/b", "test/nest/a/b");
    }
}
