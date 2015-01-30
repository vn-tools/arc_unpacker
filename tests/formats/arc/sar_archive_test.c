#include "formats/arc/sar_archive.h"
#include "test_support/archive_support.h"

void test_sar_archive()
{
    Array *expected_files = array_create();
    VirtualFile *file1 = vf_create();
    VirtualFile *file2 = vf_create();
    vf_set_name(file1, "abc.txt");
    vf_set_data(file1, "123", 3);
    vf_set_name(file2, "dir/another.txt");
    vf_set_data(file2, "AAAAAAAAAAAAAAAA", 16);
    array_set(expected_files, 0, file1);
    array_set(expected_files, 1, file2);

    Archive *archive = sar_archive_create();

    OutputFiles *output_files = unpack_to_memory(
        "tests/test_files/sar/test.sar",
        archive,
        0,
        NULL);
    Array *actual_files = output_files_get_saved(output_files);

    compare_files(expected_files, actual_files);
    output_files_destroy(output_files);

    vf_destroy(file1);
    vf_destroy(file2);
    archive_destroy(archive);
    array_destroy(expected_files);
}

int main(void)
{
    test_sar_archive();
    return 0;
}
