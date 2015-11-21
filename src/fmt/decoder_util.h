#pragma once

#include "fmt/registry.h"
#include "file_saver.h"

namespace au {
namespace fmt {

    void unpack_recursive(
        const std::vector<std::string> &arguments,
        IDecoder &decoder,
        io::File &file,
        const FileSaver &file_saver,
        const Registry &registry);

    void unpack_non_recursive(
        const std::vector<std::string> &arguments,
        IDecoder &decoder,
        io::File &file,
        const FileSaver &file_saver);

} }
