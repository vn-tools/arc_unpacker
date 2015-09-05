#pragma once

#include "fmt/kirikiri/xp3_filter.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace xp3_filters {

    class MixedXor final : public Xp3Filter
    {
    public:
        virtual void decode(File &file, u32 key) const override;
    };

} } } }
