#pragma once

#include "file_saver.h"
#include "fmt/idecoder.h"
#include "fmt/registry.h"

namespace au {
namespace fmt {

    io::path decorate_path(
        const IDecoder::NamingStrategy strategy,
        const io::path &parent_name,
        const io::path &current_name);

    void unpack_recursive(
        const Logger &logger,
        const std::vector<std::string> &arguments,
        IDecoder &decoder,
        io::File &file,
        const FileSaver &file_saver,
        const Registry &registry);

    void unpack_non_recursive(
        const Logger &logger,
        const std::vector<std::string> &arguments,
        IDecoder &decoder,
        io::File &file,
        const FileSaver &file_saver);

} }
