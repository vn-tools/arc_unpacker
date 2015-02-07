#include <cassert>
#include "arg_parser.h"
#include "file_io.h"
#include "logger.h"
#include "test_support/archive_support.h"
#include "virtual_file.h"

std::unique_ptr<OutputFilesMemory> unpack_to_memory(
    const std::string input_path,
    Archive &archive,
    int argc,
    const char **argv)
{
    log_save();
    log_silence();

    ArgParser arg_parser;
    arg_parser.parse(argc, argv);
    FileIO io(input_path.c_str(), "rb");
    std::unique_ptr<OutputFilesMemory> output_files(new OutputFilesMemory);
    archive.unpack(io, *output_files);

    log_restore();
    return output_files;
}

void compare_files(
    const std::vector<VirtualFile*> &expected_files,
    const std::vector<VirtualFile*> &actual_files)
{
    assert(actual_files.size() == expected_files.size());
    for (size_t i = 0; i < expected_files.size(); i ++)
    {
        auto *expected_file = expected_files[i];
        auto *actual_file = actual_files[i];
        assert(expected_file->name == actual_file->name);
        assert(expected_file->io.size() == actual_file->io.size());
        expected_file->io.seek(0);
        actual_file->io.seek(0);
        for (size_t j = 0; j < expected_file->io.size(); j ++)
            assert(expected_file->io.read_u8() == actual_file->io.read_u8());
    }
}
