#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace tanuki_soft {

    class TacArchive final : public Archive
    {
    public:
        TacArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }
