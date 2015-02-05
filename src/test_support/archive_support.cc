#include <cassert>
#include <cstring>
#include "arg_parser.h"
#include "io.h"
#include "logger.h"
#include "test_support/archive_support.h"
#include "virtual_file.h"

OutputFiles *unpack_to_memory(
    const char *input_path,
    Archive *archive,
    int argc,
    const char **argv)
{
    log_save();
    log_silence();

    ArgParser arg_parser;
    arg_parser.parse(argc, argv);
    IO *io = io_create_from_file(input_path, "rb");
    assert(io != NULL);
    OutputFiles *output_files = output_files_create_memory();
    archive_unpack(archive, io, output_files);
    io_destroy(io);

    log_restore();
    return output_files;
}

void compare_files(Array *expected_files, Array *actual_files)
{
    VirtualFile *expected_file;
    VirtualFile *actual_file;
    size_t i, j;
    assert(array_size(expected_files) == array_size(actual_files));
    for (i = 0; i < array_size(expected_files); i ++)
    {
        expected_file = (VirtualFile*)array_get(expected_files, i);
        actual_file = (VirtualFile*)array_get(actual_files, i);
        assert(strcmp(
            virtual_file_get_name(expected_file),
            virtual_file_get_name(actual_file)) == 0);
        assert(io_size(expected_file->io) == io_size(actual_file->io));
        io_seek(expected_file->io, 0);
        io_seek(actual_file->io, 0);
        for (j = 0; j < io_size(expected_file->io); j ++)
        {
            assert(
                io_read_u8(expected_file->io) == io_read_u8(actual_file->io));
        }
    }
}
