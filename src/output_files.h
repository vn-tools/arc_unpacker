#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include "options.h"
#include "virtual_file.h"

typedef struct OutputFiles OutputFiles;

OutputFiles *output_files_create(const Options *const options);

void output_files_destroy(OutputFiles *output_files);

bool output_files_save(
    OutputFiles *output_files,
    VirtualFile *(*save_proc)(void *),
    void *context);

#endif
