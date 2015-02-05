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
    Array *expected_files = array_create();
    VirtualFile *file1 = virtual_file_create();
    VirtualFile *file2 = virtual_file_create();
    virtual_file_set_name(file1, "another.txt");
    virtual_file_set_name(file2, "abc.txt");
    io_write_string(file1->io, "abcdefghij", 10);
    io_write_string(file2->io, "123", 3);
    array_set(expected_files, 0, file1);
    array_set(expected_files, 1, file2);

    Archive *archive = new RpaArchive();

    OutputFiles *output_files = unpack_to_memory(
        path,
        archive,
        0,
        nullptr);
    Array *actual_files = output_files_get_saved(output_files);

    compare_files(expected_files, actual_files);
    output_files_destroy(output_files);

    virtual_file_destroy(file1);
    virtual_file_destroy(file2);
    delete archive;
    array_destroy(expected_files);
}

int main(void)
{
    test_rpa_archive("tests/test_files/arc/rpa/v3.rpa");
    test_rpa_archive("tests/test_files/arc/rpa/v2.rpa");
    test_rpa_archive("tests/test_files/arc/rpa/prefixes.rpa");
    return 0;
}
