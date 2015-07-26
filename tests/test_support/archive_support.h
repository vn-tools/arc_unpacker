#ifndef AU_TEST_SUPPORT_ARCHIVE_SUPPORT_H
#define AU_TEST_SUPPORT_ARCHIVE_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <vector>
#include "fmt/archive.h"
#include "file_saver.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<File>> unpack_to_memory(
        const boost::filesystem::path &input_path, fmt::Archive &archive);

    void compare_files(
        const std::vector<std::shared_ptr<File>> &expected_files,
        const std::vector<std::shared_ptr<File>> &actual_files);

} }

#endif
