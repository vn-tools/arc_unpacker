#include <boost/filesystem/path.hpp>
#include "fmt/transformer.h"
#include "test_support/catch.hpp"

using path = boost::filesystem::path;

static path test(
    const au::fmt::FileNamingStrategy &strategy,
    const std::string &parent_path,
    const std::string &child_path)
{
    return path(au::fmt::FileNameDecorator::decorate(
        strategy, parent_path, child_path));
}

TEST_CASE("File naming strategies work")
{
    auto root    = au::fmt::FileNamingStrategy::Root;
    auto sibling = au::fmt::FileNamingStrategy::Sibling;
    auto child   = au::fmt::FileNamingStrategy::Child;
    REQUIRE(test(root,    "",           "file") == path("file"));
    REQUIRE(test(root,    "test",       "file") == path("file"));
    REQUIRE(test(root,    "test/",      "file") == path("file"));
    REQUIRE(test(root,    "test/nest",  "file") == path("file"));
    REQUIRE(test(root,    "test/nest/", "file") == path("file"));
    REQUIRE(test(sibling, "",           "file") == path("file"));
    REQUIRE(test(sibling, "test",       "file") == path("file"));
    REQUIRE(test(sibling, "test/",      "file") == path("test/file"));
    REQUIRE(test(sibling, "test/nest",  "file") == path("test/file"));
    REQUIRE(test(sibling, "test/nest/", "file") == path("test/nest/file"));
    REQUIRE(test(child,   "",           "file") == path("file"));
    REQUIRE(test(child,   "test",       "file") == path("test/file"));
    REQUIRE(test(child,   "test/",      "file") == path("test/file"));
    REQUIRE(test(child,   "test/nest",  "file") == path("test/nest/file"));
    REQUIRE(test(child,   "test/nest/", "file") == path("test/nest/file"));

    REQUIRE(test(root,    "test/nest",  "a/b") == path("a/b"));
    REQUIRE(test(sibling, "test/nest",  "a/b") == path("test/a/b"));
    REQUIRE(test(child,   "test/nest",  "a/b") == path("test/nest/a/b"));
}
