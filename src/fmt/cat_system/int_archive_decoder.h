#pragma once

#include "fmt/base_archive_decoder.h"

namespace au {
namespace fmt {
namespace cat_system {

    class IntArchiveDecoder final : public BaseArchiveDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;
    };

} } }
