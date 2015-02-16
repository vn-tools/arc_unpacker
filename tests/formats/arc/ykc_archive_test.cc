#include "formats/arc/ykc_archive.h"
#include "test_support/archive_support.h"

void test_ykc_archive()
{
    std::unique_ptr<File> file1(new File);
    std::unique_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghij", 10);
    std::vector<File*> expected_files { file1.get(), file2.get() };

    std::string path = "tests/test_files/arc/ykc/test.ykc";
    std::unique_ptr<Archive> archive(new YkcArchive);
    auto output_files = unpack_to_memory(path, *archive);
    auto actual_files = output_files->get_saved();

    compare_files(expected_files, actual_files);
}

int main(void)
{
    test_ykc_archive();
    return 0;
}
