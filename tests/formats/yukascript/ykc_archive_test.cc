#include "formats/yukascript/ykc_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"
using namespace Formats::YukaScript;

TEST_CASE("Unpacking YKC archives works")
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghij", 10);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new YkcArchive);
    compare_files(
        expected_files,
        unpack_to_memory("tests/formats/yukascript/files/test.ykc", *archive));
}
