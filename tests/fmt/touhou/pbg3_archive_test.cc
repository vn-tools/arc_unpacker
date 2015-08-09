#include "fmt/touhou/pbg3_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::touhou;

TEST_CASE("Unpacking PBG3 archives works")
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123"_b);
    file2->io.write("abcdefghijaaabcd"_b);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new Pbg3Archive);
    au::tests::compare_files(
        expected_files,
        au::tests::unpack_to_memory(
            "tests/fmt/touhou/files/test.pbg3", *archive));
}
