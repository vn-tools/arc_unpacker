#ifndef OUTPUT_FILES_H
#define OUTPUT_FILES_H
#include <vector>
#include "virtual_file.h"

typedef struct OutputFiles OutputFiles;

OutputFiles *output_files_create_hdd(const char *output_dir);
OutputFiles *output_files_create_memory();

void output_files_destroy(OutputFiles *output_files);

bool output_files_save(
    OutputFiles *output_files,
    VirtualFile *(*save_proc)(void *),
    void *context);

std::vector<VirtualFile*> output_files_get_saved(
    const OutputFiles *output_files);

#endif
