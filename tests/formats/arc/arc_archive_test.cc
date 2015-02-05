#include "formats/arc/arc_archive.h"
#include "test_support/archive_support.h"

int main(void)
{
    Array *expected_files = array_create();
    VirtualFile *file1 = virtual_file_create();
    VirtualFile *file2 = virtual_file_create();
    virtual_file_set_name(file1, "abc.txt");
    virtual_file_set_name(file2, "another.txt");
    io_write_string(file1->io, "123", 3);
    io_write_string(file2->io, "abcdefghij", 10);
    array_set(expected_files, 0, file1);
    array_set(expected_files, 1, file2);

    const char *path = "tests/test_files/arc/arc/test.arc";
    Archive *archive = new ArcArchive();
    OutputFiles *output_files = unpack_to_memory(path, archive, 0, nullptr);
    Array *actual_files = output_files_get_saved(output_files);

    compare_files(expected_files, actual_files);
    output_files_destroy(output_files);

    virtual_file_destroy(file1);
    virtual_file_destroy(file2);
    delete archive;
    array_destroy(expected_files);
    return 0;
}
