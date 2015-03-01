#ifndef FORMATS_KIRIKIRI_XP3_FILTERS_NOOP_FILTER_H
#define FORMATS_KIRIKIRI_XP3_FILTERS_NOOP_FILTER_H
#include "formats/kirikiri/xp3_filters/filter.h"

namespace Formats
{
    namespace Kirikiri
    {
        namespace Xp3Filters
        {
            class NoopFilter final : public Filter
            {
            public:
                virtual void decode(File &file, uint32_t key) const override;
            };
        }
    }
}

#endif
