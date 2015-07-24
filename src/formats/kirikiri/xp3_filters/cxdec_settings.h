#ifndef FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_SETTINGS_H
#define FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_SETTINGS_H
#include <array>
#include "types.h"

namespace Formats
{
    namespace Kirikiri
    {
        namespace Xp3Filters
        {
            class CxdecSettings
            {
            public:
                u16 key1;
                u16 key2;
                std::array<size_t, 3> key_derivation_order1;
                std::array<size_t, 8> key_derivation_order2;
                std::array<size_t, 6> key_derivation_order3;
            };
        }
    }
}

#endif
