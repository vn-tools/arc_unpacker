#pragma once

#include "fmt/registry.h"
#include "io/file.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<io::File>> flow_unpack(
        const fmt::Registry &registry,
        const bool enable_ensted_decoding,
        io::File &input_file);

} }
