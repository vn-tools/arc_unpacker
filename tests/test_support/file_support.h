#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include "file.h"

namespace au {
namespace tests {

    std::shared_ptr<File> stub_file(
        const std::string &name, const bstr &data);

    std::shared_ptr<File> file_from_path(
        const boost::filesystem::path &path, const std::string &cust_name = "");

    std::shared_ptr<File> zlib_file_from_path(
        const boost::filesystem::path &path, const std::string &cust_name = "");

    void compare_files(
        const std::vector<std::shared_ptr<File>> &expected_files,
        const std::vector<std::shared_ptr<File>> &actual_files,
        const bool compare_file_names);

    void compare_files(
        const File &expected_file,
        const File &actual_file,
        const bool compare_file_names);

} }
