#include "formats/arc/pak_archive.h"
#include "test_support/archive_support.h"

void test_pak_archive(const std::string path)
{
    std::vector<File*> expected_files;
    std::unique_ptr<File> file1(new File);
    std::unique_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghij", 10);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::unique_ptr<Archive> archive(new PakArchive);
    auto output_files = unpack_to_memory(path, *archive);
    auto actual_files = output_files->get_saved();

    compare_files(expected_files, actual_files);
}

int main(void)
{
    test_pak_archive("tests/test_files/arc/pak/uncompressed.pak");
    test_pak_archive("tests/test_files/arc/pak/compressed.pak");
    return 0;
}
