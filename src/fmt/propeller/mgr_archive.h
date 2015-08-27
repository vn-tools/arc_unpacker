#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace propeller {

    class MgrArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }
