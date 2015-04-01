#ifndef TEST_SUPPORT_ARCHIVE_SUPPORT_H
#define TEST_SUPPORT_ARCHIVE_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <vector>
#include "formats/archive.h"
#include "file_saver.h"

std::vector<std::shared_ptr<File>> unpack_to_memory(
    const boost::filesystem::path &input_path, Archive &archive);

void compare_files(
    const std::vector<std::shared_ptr<File>> &expected_files,
    const std::vector<std::shared_ptr<File>> &actual_files);

#endif
