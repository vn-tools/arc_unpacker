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
            class Cxdec : public Xp3Filter
            {
            public:
                Cxdec();
                ~Cxdec();
                virtual void decode(File &file, u32 key) const override;
                virtual CxdecSettings get_settings() const = 0;
                void set_arc_path(const std::string &path) override;
            private:
                struct Priv;
                std::unique_ptr<Priv> p;
            };
        }
    }
}

#endif
