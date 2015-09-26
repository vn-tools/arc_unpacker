#include <boost/filesystem/path.hpp>
#include "fmt/naming_strategies.h"
#include "test_support/catch.hh"

using namespace au::fmt;
using path = boost::filesystem::path;

static path test(
    const INamingStrategy &strategy,
    const std::string &parent_path,
    const std::string &child_path)
{
    return path(strategy.decorate(parent_path, child_path));
}

TEST_CASE("File naming strategies", "[fmt_core]")
{
    RootNamingStrategy    root;
    SiblingNamingStrategy sibling;
    ChildNamingStrategy   child;

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
