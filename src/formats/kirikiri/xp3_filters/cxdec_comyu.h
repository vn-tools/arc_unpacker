#ifndef FORMATS_KIRIKIRI_XP3_FILTERS_COMYU_H
#define FORMATS_KIRIKIRI_XP3_FILTERS_COMYU_H
#include "formats/kirikiri/xp3_filters/cxdec.h"

namespace Formats
{
    namespace Kirikiri
    {
        namespace Xp3Filters
        {
            class CxdecComyu final : public Cxdec
            {
            public:
                virtual CxdecSettings get_settings() const override;
            };
        }
    }
}

#endif
