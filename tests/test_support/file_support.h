#ifndef AU_TESTS_TEST_SUPPORT_FILE_SUPPORT_H
#define AU_TESTS_TEST_SUPPORT_FILE_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include "file.h"

namespace au {
namespace tests {

    std::shared_ptr<File> file_from_path(const boost::filesystem::path &path);

    std::shared_ptr<File> create_file(
        const std::string &name, const bstr &data);

    void compare_files(
        const std::vector<std::shared_ptr<File>> &expected_files,
        const std::vector<std::shared_ptr<File>> &actual_files,
        bool compare_file_names);

    void compare_files(
        const File &expected_file,
        const File &actual_file,
        bool compare_file_names);

} }

#endif
