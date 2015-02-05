#include "formats/arc/mbl_archive.h"
#include "test_support/archive_support.h"

void test_mbl_archive(const char *path)
{
    Array *expected_files = array_create();
    VirtualFile *file1 = virtual_file_create();
    VirtualFile *file2 = virtual_file_create();
    virtual_file_set_name(file1, "abc.txt");
    virtual_file_set_name(file2, "テスト");
    io_write_string(file1->io, "abc", 3);
    io_write_string(file2->io, "AAAAAAAAAAAAAAAA", 16);
    array_set(expected_files, 0, file1);
    array_set(expected_files, 1, file2);

    Archive *archive = mbl_archive_create();

    OutputFiles *output_files = unpack_to_memory(path, archive, 0, nullptr);
    Array *actual_files = output_files_get_saved(output_files);

    compare_files(expected_files, actual_files);
    output_files_destroy(output_files);

    virtual_file_destroy(file1);
    virtual_file_destroy(file2);
    archive_destroy(archive);
    array_destroy(expected_files);
}

int main(void)
{
    test_mbl_archive("tests/test_files/arc/mbl/mbl-v1.mbl");
    test_mbl_archive("tests/test_files/arc/mbl/mbl-v2.mbl");
    return 0;
}
