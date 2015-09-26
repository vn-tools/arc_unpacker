#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace tanuki_soft {

    class TacArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TacArchiveDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }
