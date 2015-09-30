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
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
        std::unique_ptr<INamingStrategy> naming_strategy() const override;
    };

} } }
