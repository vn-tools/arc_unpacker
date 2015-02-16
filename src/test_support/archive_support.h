#ifndef TEST_SUPPORT_ARCHIVE_SUPPORT_H
#define TEST_SUPPORT_ARCHIVE_SUPPORT_H
#include <memory>
#include <string>
#include <vector>
#include "formats/archive.h"
#include "file_saver.h"

std::unique_ptr<FileSaverMemory> unpack_to_memory(
    const std::string input_path, Archive &archive);

void compare_files(
    const std::vector<File*> &expected_files,
    const std::vector<File*> &actual_files);

#endif
