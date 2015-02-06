#include "formats/arc/xp3_archive.h"
#include "test_support/archive_support.h"

void test_xp3_archive(const char *path)
{
    std::vector<VirtualFile*> expected_files;
    std::unique_ptr<VirtualFile> file1(new VirtualFile);
    std::unique_ptr<VirtualFile> file2(new VirtualFile);
    file1->name = "abc.txt";
    file2->name = "abc2.txt";
    io_write_string(&file1->io, "123", 3);
    io_write_string(&file2->io, "AAAAAAAAAA", 10);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::unique_ptr<Archive> archive(new Xp3Archive);
    auto output_files = unpack_to_memory(path, *archive, 0, nullptr);
    auto actual_files = output_files->get_saved();

    compare_files(expected_files, actual_files);
}

int main(void)
{
    test_xp3_archive("tests/test_files/arc/xp3/xp3-v1.xp3");
    test_xp3_archive("tests/test_files/arc/xp3/xp3-v2.xp3");
    test_xp3_archive("tests/test_files/arc/xp3/xp3-compressed-table.xp3");
    test_xp3_archive("tests/test_files/arc/xp3/xp3-compressed-files.xp3");
    test_xp3_archive("tests/test_files/arc/xp3/xp3-multiple-segm.xp3");
    return 0;
}
