#include "collections/array.h"
#include "formats/archive.h"
#include "output_files.h"

OutputFiles *unpack_to_memory(
    const char *input_path,
    Archive *archive,
    int argc,
    const char **argv);

void compare_files(Array *expected_files, Array *actual_files);
