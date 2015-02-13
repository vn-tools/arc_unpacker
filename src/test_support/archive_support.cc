#include <cassert>
#include "arg_parser.h"
#include "file_io.h"
#include "test_support/archive_support.h"
#include "test_support/eassert.h"
#include "virtual_file.h"

std::unique_ptr<OutputFilesMemory> unpack_to_memory(
    const std::string input_path, Archive &archive)
{
    ArgParser arg_parser;
    VirtualFile file(input_path, "rb");
    std::unique_ptr<OutputFilesMemory> output_files(new OutputFilesMemory);
    archive.unpack(file, *output_files);
    return output_files;
}

void compare_files(
    const std::vector<VirtualFile*> &expected_files,
    const std::vector<VirtualFile*> &actual_files)
{
    eassert(actual_files.size() == expected_files.size());
    for (size_t i = 0; i < expected_files.size(); i ++)
    {
        auto *expected_file = expected_files[i];
        auto *actual_file = actual_files[i];
        eassert(expected_file->name == actual_file->name);
        eassert(expected_file->io.size() == actual_file->io.size());
        expected_file->io.seek(0);
        actual_file->io.seek(0);
        for (size_t j = 0; j < expected_file->io.size(); j ++)
            eassert(expected_file->io.read_u8() == actual_file->io.read_u8());
    }
}
