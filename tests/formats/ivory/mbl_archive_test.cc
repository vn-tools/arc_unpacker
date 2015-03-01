#include "formats/ivory/mbl_archive.h"
#include "test_support/archive_support.h"
using namespace Formats::Ivory;

void test_mbl_archive(const char *path)
{
    std::vector<File*> expected_files;
    std::unique_ptr<File> file1(new File);
    std::unique_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "テスト";
    file1->io.write("abc", 3);
    file2->io.write("AAAAAAAAAAAAAAAA", 16);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::unique_ptr<Archive> archive(new MblArchive);
    compare_files(expected_files, unpack_to_memory(path, *archive));
}

int main(void)
{
    test_mbl_archive("tests/formats/ivory/files/mbl-v1.mbl");
    test_mbl_archive("tests/formats/ivory/files/mbl-v2.mbl");
    return 0;
}
