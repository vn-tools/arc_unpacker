#include "formats/arc/mbl_archive.h"
#include "test_support/archive_support.h"

void test_mbl_archive(const char *path)
{
    std::vector<VirtualFile*> expected_files;
    std::unique_ptr<VirtualFile> file1(new VirtualFile);
    std::unique_ptr<VirtualFile> file2(new VirtualFile);
    file1->name = "abc.txt";
    file2->name = "テスト";
    file1->io.write("abc", 3);
    file2->io.write("AAAAAAAAAAAAAAAA", 16);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::unique_ptr<Archive> archive(new MblArchive);
    auto output_files = unpack_to_memory(path, *archive, 0, nullptr);
    auto actual_files = output_files->get_saved();

    compare_files(expected_files, actual_files);
}

int main(void)
{
    test_mbl_archive("tests/test_files/arc/mbl/mbl-v1.mbl");
    test_mbl_archive("tests/test_files/arc/mbl/mbl-v2.mbl");
    return 0;
}
