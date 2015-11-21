#include "fmt/naming_strategies.h"
#include "io/path.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

static io::path test(
    const INamingStrategy &strategy,
    const std::string &parent_path,
    const std::string &child_path)
{
    return io::path(strategy.decorate(parent_path, child_path));
}

TEST_CASE("File naming strategies", "[fmt_core]")
{
    RootNamingStrategy    root;
    SiblingNamingStrategy sibling;
    ChildNamingStrategy   child;

    REQUIRE(test(root,    "",           "file") == io::path("file"));
    REQUIRE(test(root,    "test",       "file") == io::path("file"));
    REQUIRE(test(root,    "test/",      "file") == io::path("file"));
    REQUIRE(test(root,    "test/nest",  "file") == io::path("file"));
    REQUIRE(test(root,    "test/nest/", "file") == io::path("file"));
    REQUIRE(test(sibling, "",           "file") == io::path("file"));
    REQUIRE(test(sibling, "test",       "file") == io::path("file"));
    REQUIRE(test(sibling, "test/",      "file") == io::path("test/file"));
    REQUIRE(test(sibling, "test/nest",  "file") == io::path("test/file"));
    REQUIRE(test(sibling, "test/nest/", "file") == io::path("test/nest/file"));
    REQUIRE(test(child,   "",           "file") == io::path("file"));
    REQUIRE(test(child,   "test",       "file") == io::path("test/file"));
    REQUIRE(test(child,   "test/",      "file") == io::path("test/file"));
    REQUIRE(test(child,   "test/nest",  "file") == io::path("test/nest/file"));
    REQUIRE(test(child,   "test/nest/", "file") == io::path("test/nest/file"));

    REQUIRE(test(root,    "test/nest",  "a/b") == io::path("a/b"));
    REQUIRE(test(sibling, "test/nest",  "a/b") == io::path("test/a/b"));
    REQUIRE(test(child,   "test/nest",  "a/b") == io::path("test/nest/a/b"));
}
