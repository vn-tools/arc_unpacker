#include "formats/renpy/rpa_archive.h"
#include "test_support/archive_support.h"
using namespace Formats::Renpy;

// import cPickle
// import zlib
//
// key = 0
// def filter(x):
//   return x ^ key
//
// print zlib.compress(cPickle.dumps({
//   'abc.txt': [(filter(0x19), filter(2), '1')],
//   'another.txt': [(filter(0x1B), filter(7), 'abc')]
// }, cPickle.HIGHEST_PROTOCOL))

void test_rpa_archive(const char *path)
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "another.txt";
    file2->name = "abc.txt";
    file1->io.write("abcdefghij", 10);
    file2->io.write("123", 3);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new RpaArchive);
    compare_files(expected_files, unpack_to_memory(path, *archive));
}

int main(void)
{
    test_rpa_archive("tests/formats/renpy/files/v3.rpa");
    test_rpa_archive("tests/formats/renpy/files/v2.rpa");
    test_rpa_archive("tests/formats/renpy/files/prefixes.rpa");
    return 0;
}
