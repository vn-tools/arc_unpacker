#pragma once

#include <memory>
#include "io/file.h"

namespace au {
namespace tests {

    std::shared_ptr<io::File> stub_file(
        const std::string &name, const bstr &data);

    std::shared_ptr<io::File> file_from_path(
        const io::path &path, const std::string &cust_name = "");

    std::shared_ptr<io::File> zlib_file_from_path(
        const io::path &path, const std::string &cust_name = "");

    void compare_files(
        const std::vector<std::shared_ptr<io::File>> &expected_files,
        const std::vector<std::shared_ptr<io::File>> &actual_files,
        const bool compare_file_names);

    void compare_files(
        const io::File &expected_file,
        const io::File &actual_file,
        const bool compare_file_names);

} }
