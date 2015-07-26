#ifndef AU_FMT_KIRIKIRI_XP3_FILTERS_NOOP_H
#define AU_FMT_KIRIKIRI_XP3_FILTERS_NOOP_H
#include "fmt/kirikiri/xp3_filter.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace xp3_filters {

    class Noop final : public Xp3Filter
    {
    public:
        virtual void decode(File &file, u32 key) const override;
    };

} } } }

#endif
