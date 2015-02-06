#include <cassert>
#include "arg_parser.h"
#include "io.h"
#include "logger.h"
#include "test_support/archive_support.h"
#include "virtual_file.h"

std::unique_ptr<OutputFilesMemory> unpack_to_memory(
    std::string input_path,
    Archive &archive,
    int argc,
    const char **argv)
{
    log_save();
    log_silence();

    ArgParser arg_parser;
    arg_parser.parse(argc, argv);
    IO *io = io_create_from_file(input_path.c_str(), "rb");
    assert(io != nullptr);
    std::unique_ptr<OutputFilesMemory> output_files(new OutputFilesMemory);
    archive.unpack(io, *output_files);
    io_destroy(io);

    log_restore();
    return output_files;
}

void compare_files(
    const std::vector<VirtualFile*> &expected_files,
    const std::vector<VirtualFile*> &actual_files)
{
    VirtualFile *expected_file;
    VirtualFile *actual_file;
    assert(actual_files.size() == expected_files.size());
    for (size_t i = 0; i < expected_files.size(); i ++)
    {
        expected_file = expected_files[i];
        actual_file = actual_files[i];
        assert(expected_file->name == actual_file->name);
        assert(io_size(&expected_file->io) == io_size(&actual_file->io));
        io_seek(&expected_file->io, 0);
        io_seek(&actual_file->io, 0);
        for (size_t j = 0; j < io_size(&expected_file->io); j ++)
        {
            assert(
                io_read_u8(&expected_file->io) == io_read_u8(&actual_file->io));
        }
    }
}
