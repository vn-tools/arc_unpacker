#ifndef TEST_SUPPORT_ARCHIVE_SUPPORT_H
#define TEST_SUPPORT_ARCHIVE_SUPPORT_H
#include <memory>
#include <string>
#include <vector>
#include "formats/archive.h"
#include "output_files.h"

std::unique_ptr<OutputFilesMemory> unpack_to_memory(
    const std::string input_path, Archive &archive);

void compare_files(
    const std::vector<VirtualFile*> &expected_files,
    const std::vector<VirtualFile*> &actual_files);

#endif
