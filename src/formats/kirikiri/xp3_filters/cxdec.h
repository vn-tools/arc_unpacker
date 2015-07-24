#ifndef FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_H
#define FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_H
#include <memory>
#include "formats/kirikiri/xp3_filters/cxdec_settings.h"
#include "formats/kirikiri/xp3_filter.h"

namespace Formats
{
    namespace Kirikiri
    {
        namespace Xp3Filters
        {
            class Cxdec final : public Xp3Filter
            {
            public:
                Cxdec(CxdecSettings &);
                ~Cxdec();
                virtual void decode(File &file, u32 key) const override;
            private:
                struct Priv;
                std::unique_ptr<Priv> p;
            };
        }
    }
}

#endif
