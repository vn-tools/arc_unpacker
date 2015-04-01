#include "formats/ivory/mbl_archive.h"
#include "test_support/archive_support.h"
using namespace Formats::Ivory;

void test_mbl_archive(const char *path)
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "テスト";
    file1->io.write("abc", 3);
    file2->io.write("AAAAAAAAAAAAAAAA", 16);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new MblArchive);
    compare_files(expected_files, unpack_to_memory(path, *archive));
}

int main(void)
{
    test_mbl_archive("tests/formats/ivory/files/mbl-v1.mbl");
    test_mbl_archive("tests/formats/ivory/files/mbl-v2.mbl");
    return 0;
}
