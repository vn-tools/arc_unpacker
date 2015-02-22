#include "formats/arc/pbg4_archive.h"
#include "test_support/archive_support.h"

void test_pbg4_archive()
{
    std::vector<File*> expected_files;
    std::unique_ptr<File> file1(new File);
    std::unique_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "another.txt";
    file1->io.write("123", 3);
    file2->io.write("abcdefghijaaabcd", 16);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::string path = "tests/test_files/arc/pbg4/test.pbg4";
    std::unique_ptr<Archive> archive(new Pbg4Archive);
    compare_files(expected_files,  unpack_to_memory(path, *archive));
}

int main(void)
{
    test_pbg4_archive();
    return 0;
}
