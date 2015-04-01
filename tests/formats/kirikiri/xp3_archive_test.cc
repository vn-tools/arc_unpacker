#include "formats/kirikiri/xp3_archive.h"
#include "test_support/archive_support.h"
using namespace Formats::Kirikiri;

void test_xp3_archive(const char *path)
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "abc2.txt";
    file1->io.write("123", 3);
    file2->io.write("AAAAAAAAAA", 10);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new Xp3Archive);
    compare_files(expected_files, unpack_to_memory(path, *archive));
}

int main(void)
{
    test_xp3_archive("tests/formats/kirikiri/files/xp3-v1.xp3");
    test_xp3_archive("tests/formats/kirikiri/files/xp3-v2.xp3");
    test_xp3_archive("tests/formats/kirikiri/files/xp3-compressed-table.xp3");
    test_xp3_archive("tests/formats/kirikiri/files/xp3-compressed-files.xp3");
    test_xp3_archive("tests/formats/kirikiri/files/xp3-multiple-segm.xp3");
    return 0;
}
