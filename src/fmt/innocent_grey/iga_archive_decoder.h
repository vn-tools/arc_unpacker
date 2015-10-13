#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace innocent_grey {

    class IgaArchiveDecoder final : public ArchiveDecoder
    {
    protected:
        bool is_recognized_impl(File &) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    };

} } }
