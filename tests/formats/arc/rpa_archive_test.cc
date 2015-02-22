#include "formats/arc/rpa_archive.h"
#include "test_support/archive_support.h"

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
    std::vector<File*> expected_files;
    std::unique_ptr<File> file1(new File);
    std::unique_ptr<File> file2(new File);
    file1->name = "another.txt";
    file2->name = "abc.txt";
    file1->io.write("abcdefghij", 10);
    file2->io.write("123", 3);
    expected_files.push_back(file1.get());
    expected_files.push_back(file2.get());

    std::unique_ptr<Archive> archive(new RpaArchive);
    compare_files(expected_files, unpack_to_memory(path, *archive));
}

int main(void)
{
    test_rpa_archive("tests/test_files/arc/rpa/v3.rpa");
    test_rpa_archive("tests/test_files/arc/rpa/v2.rpa");
    test_rpa_archive("tests/test_files/arc/rpa/prefixes.rpa");
    return 0;
}
