#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <vector>
#include "fmt/archive_decoder.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<File>> unpack_to_memory(
        const boost::filesystem::path &input_path,
        fmt::ArchiveDecoder &decoder);

} }
