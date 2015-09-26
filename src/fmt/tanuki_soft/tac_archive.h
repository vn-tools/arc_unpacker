#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace tanuki_soft {

    class TacArchive final : public ArchiveDecoder
    {
    public:
        TacArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }
