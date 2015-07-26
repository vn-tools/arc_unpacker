#include "formats/bgi/arc_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::bgi;

TEST_CASE("Unpacking ARC archives works")
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghij", 10);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::string path = "tests/formats/bgi/files/test.arc";
    std::unique_ptr<Archive> archive(new ArcArchive);
    au::tests::compare_files(
        expected_files,
        au::tests::unpack_to_memory(path, *archive));
}
