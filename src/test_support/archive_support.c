#include "arg_parser.h"
#include "assert_ex.h"
#include "io.h"
#include "test_support/archive_support.h"
#include "virtual_file.h"

OutputFiles *unpack_to_memory(
    const char *input_path,
    Archive *archive,
    int argc,
    const char **argv)
{
    ArgParser *arg_parser = arg_parser_create();
    arg_parser_parse(arg_parser, argc, argv);
    IO *io = io_create_from_file(input_path, "rb");
    assert_not_null(io);
    OutputFiles *output_files = output_files_create_memory();
    archive_unpack(archive, io, output_files);
    io_destroy(io);
    arg_parser_destroy(arg_parser);
    return output_files;
}

void compare_files(Array *expected_files, Array *actual_files)
{
    VirtualFile *expected_file;
    VirtualFile *actual_file;
    size_t i;
    assert_equali(array_size(expected_files), array_size(actual_files));
    for (i = 0; i < array_size(expected_files); i ++)
    {
        expected_file = (VirtualFile*)array_get(expected_files, i);
        actual_file = (VirtualFile*)array_get(actual_files, i);
        assert_equals(vf_get_name(expected_file), vf_get_name(actual_file));
        assert_equali(vf_get_size(expected_file), vf_get_size(actual_file));
        assert_equalsn(
            vf_get_data(expected_file),
            vf_get_data(actual_file),
            vf_get_size(expected_file));
    }
}
