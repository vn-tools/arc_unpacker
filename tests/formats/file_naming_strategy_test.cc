#include <boost/filesystem/path.hpp>
#include "formats/file_naming_strategy.h"
#include "test_support/eassert.h"

typedef boost::filesystem::path path;

path test(
    const FileNamingStrategy &strategy,
    const std::string &parent_path,
    const std::string &child_path)
{
    return path(FileNameDecorator::decorate(strategy, parent_path, child_path));
}

int main(void)
{
    auto root    = FileNamingStrategy::Root;
    auto sibling = FileNamingStrategy::Sibling;
    auto child   = FileNamingStrategy::Child;
    eassert(test(root,    "",           "file") == path("file"));
    eassert(test(root,    "test",       "file") == path("file"));
    eassert(test(root,    "test/",      "file") == path("file"));
    eassert(test(root,    "test/nest",  "file") == path("file"));
    eassert(test(root,    "test/nest/", "file") == path("file"));
    eassert(test(sibling, "",           "file") == path("file"));
    eassert(test(sibling, "test",       "file") == path("file"));
    eassert(test(sibling, "test/",      "file") == path("test/file"));
    eassert(test(sibling, "test/nest",  "file") == path("test/file"));
    eassert(test(sibling, "test/nest/", "file") == path("test/nest/file"));
    eassert(test(child,   "",           "file") == path("file"));
    eassert(test(child,   "test",       "file") == path("test/file"));
    eassert(test(child,   "test/",      "file") == path("test/file"));
    eassert(test(child,   "test/nest",  "file") == path("test/nest/file"));
    eassert(test(child,   "test/nest/", "file") == path("test/nest/file"));

    eassert(test(root,    "test/nest",  "a/b") == path("a/b"));
    eassert(test(sibling, "test/nest",  "a/b") == path("test/a/b"));
    eassert(test(child,   "test/nest",  "a/b") == path("test/nest/a/b"));
}
