#ifndef FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_FILTER_H
#define FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_FILTER_H
#include <memory>
#include "formats/kirikiri/xp3_filters/cxdec_filter_settings.h"
#include "formats/kirikiri/xp3_filters/filter.h"

namespace Formats
{
    namespace Kirikiri
    {
        namespace Xp3Filters
        {
            class CxdecFilter final : public Filter
            {
            public:
                CxdecFilter(CxdecFilterSettings &);
                ~CxdecFilter();
                virtual void decode(File &file, u32 key) const override;
            private:
                struct Internals;
                std::unique_ptr<Internals> internals;
            };
        }
    }
}

#endif
