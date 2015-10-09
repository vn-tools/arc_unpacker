#pragma once

#include "fmt/archive_decoder.h"
#include "pix/grid.h"

namespace au {
namespace fmt {
namespace will {

    class WipfArchiveDecoder final : public ArchiveDecoder
    {
    public:
        std::vector<std::shared_ptr<pix::Grid>> unpack_to_images(File &) const;
        std::unique_ptr<pix::Grid> read_image(
            File &, const ArchiveMeta &, const ArchiveEntry &) const;
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
        std::unique_ptr<INamingStrategy> naming_strategy() const override;
    };

} } }
