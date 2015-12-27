#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class AldArchiveDecoder final : public ArchiveDecoder
    {
    public:
        std::vector<std::string> get_linked_formats() const override;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<fmt::ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &arc_file) const;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &arc_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const;
    };

} } }
