#include "formats/arc/fjsys_archive.h"
#include "test_support/archive_support.h"

void test_fjsys_archive()
{
    std::vector<VirtualFile*> expected_files;
    VirtualFile *file1 = virtual_file_create();
    VirtualFile *file2 = virtual_file_create();
    virtual_file_set_name(file1, "abc.txt");
    virtual_file_set_name(file2, "another.txt");
    io_write_string(file1->io, "123", 3);
    io_write_string(file2->io, "abcdefghij", 10);
    expected_files.push_back(file1);
    expected_files.push_back(file2);

    Archive *archive = new FjsysArchive();

    OutputFiles *output_files = unpack_to_memory(
        "tests/test_files/arc/fjsys/test.fjsys",
        archive,
        0,
        nullptr);
    auto actual_files = output_files_get_saved(output_files);

    compare_files(expected_files, actual_files);
    output_files_destroy(output_files);

    virtual_file_destroy(file1);
    virtual_file_destroy(file2);
    delete archive;
}

int main(void)
{
    test_fjsys_archive();
    return 0;
}
