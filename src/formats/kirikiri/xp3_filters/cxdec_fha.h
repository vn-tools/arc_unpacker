#ifndef AU_FMT_KIRIKIRI_XP3_FILTERS_FHA_H
#define AU_FMT_KIRIKIRI_XP3_FILTERS_FHA_H
#include "formats/kirikiri/xp3_filters/cxdec.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace xp3_filters {

    class CxdecFha final : public Cxdec
    {
    public:
        virtual CxdecSettings get_settings() const override;
    };

} } } }

#endif
