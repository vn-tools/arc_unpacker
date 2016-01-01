#pragma once

#include "dec/registry.h"
#include "io/file.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<io::File>> flow_unpack(
        const dec::Registry &registry,
        const bool enable_ensted_decoding,
        io::File &input_file);

} }
