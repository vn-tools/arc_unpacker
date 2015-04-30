#include <boost/filesystem/path.hpp>
#include "formats/transformer.h"
#include "test_support/catch.hpp"

typedef boost::filesystem::path path;

static path test(
    const FileNamingStrategy &strategy,
    const std::string &parent_path,
    const std::string &child_path)
{
    return path(FileNameDecorator::decorate(strategy, parent_path, child_path));
}

TEST_CASE("File naming strategies work")
{
    auto root    = FileNamingStrategy::Root;
    auto sibling = FileNamingStrategy::Sibling;
    auto child   = FileNamingStrategy::Child;
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
