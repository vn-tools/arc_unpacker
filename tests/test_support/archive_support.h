#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <vector>
#include "fmt/archive.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<File>> unpack_to_memory(
        File &file, fmt::Archive &archive);

    std::vector<std::shared_ptr<File>> unpack_to_memory(
        const boost::filesystem::path &input_path, fmt::Archive &archive);

} }
