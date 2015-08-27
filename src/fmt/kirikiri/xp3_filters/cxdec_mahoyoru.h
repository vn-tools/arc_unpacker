#pragma once

#include "fmt/kirikiri/xp3_filters/cxdec.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace xp3_filters {

    class CxdecMahoYoru final : public Cxdec
    {
    public:
        virtual CxdecSettings get_settings() const override;
    };

} } } }
