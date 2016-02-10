#pragma once

#include "dec/kaguya/base_link_archive_decoder.h"

namespace au {
namespace dec {
namespace kaguya {

    class Link5ArchiveDecoder final : public BaseLinkArchiveDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        int get_version() const override;
    };

} } }
