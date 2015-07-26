#ifndef AU_FMT_KIRIKIRI_XP3_FILTERS_FSN_H
#define AU_FMT_KIRIKIRI_XP3_FSN_FILTER_H
#include "formats/kirikiri/xp3_filter.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace xp3_filters {

    class Fsn final : public Xp3Filter
    {
    public:
        virtual void decode(File &file, u32 key) const override;
    };

} } } }

#endif
