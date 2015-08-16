#ifndef AU_TEST_SUPPORT_ARCHIVE_SUPPORT_H
#define AU_TEST_SUPPORT_ARCHIVE_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <vector>
#include "fmt/archive.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<File>> unpack_to_memory(
        const boost::filesystem::path &input_path, fmt::Archive &archive);

} }

#endif
