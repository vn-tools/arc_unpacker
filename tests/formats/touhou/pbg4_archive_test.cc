#include "formats/touhou/pbg4_archive.h"
#include "test_support/archive_support.h"
using namespace Formats::Touhou;

void test_pbg4_archive()
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghijaaabcd", 16);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new Pbg4Archive);
    compare_files(
        expected_files,
        unpack_to_memory("tests/formats/touhou/files/test.pbg4", *archive));
}

int main(void)
{
    test_pbg4_archive();
    return 0;
}
