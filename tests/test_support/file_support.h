#pragma once

#include <memory>
#include "io/file.h"

namespace au {
namespace tests {

    std::unique_ptr<io::File> stub_file(
        const std::string &path, const bstr &data);

    std::unique_ptr<io::File> file_from_path(
        const io::path &path, const std::string &custom_path = "");

    std::unique_ptr<io::File> zlib_file_from_path(
        const io::path &path, const std::string &custom_path = "");

    void compare_file_paths(
        const io::path &expected_path, const io::path &actual_path);

    void compare_files(
        const io::File &expected_file,
        const io::File &actual_file,
        const bool compare_file_paths);

    void compare_files(
        const std::vector<std::shared_ptr<io::File>> &expected_files,
        const std::vector<std::shared_ptr<io::File>> &actual_files,
        const bool compare_file_paths);

} }
