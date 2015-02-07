#include "formats/arc/pak_archive.h"
#include "test_support/archive_support.h"

void test_pak_archive(const std::string path)
{
    std::vector<VirtualFile*> expected_files;
    std::unique_ptr<VirtualFile> file1(new VirtualFile);
    std::unique_ptr<VirtualFile> file2(new VirtualFile);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghij", 10);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::unique_ptr<Archive> archive(new PakArchive);
    auto output_files = unpack_to_memory(path, *archive, 0, nullptr);
    auto actual_files = output_files->get_saved();

    compare_files(expected_files, actual_files);
}

int main(void)
{
    test_pak_archive("tests/test_files/arc/pak/uncompressed.pak");
    test_pak_archive("tests/test_files/arc/pak/compressed.pak");
    return 0;
}
