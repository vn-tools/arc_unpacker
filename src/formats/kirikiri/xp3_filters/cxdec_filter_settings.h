#ifndef FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_FILTER_SETTINGS_H
#define FORMATS_KIRIKIRI_XP3_FILTERS_CXDEC_FILTER_SETTINGS_H
#include <array>

namespace Formats
{
    namespace Kirikiri
    {
        namespace Xp3Filters
        {
            class CxdecFilterSettings
            {
            public:
                uint16_t key1;
                uint16_t key2;
                std::array<size_t, 3> key_derivation_order1;
                std::array<size_t, 8> key_derivation_order2;
                std::array<size_t, 6> key_derivation_order3;
                const char *encryption_block;
            };
        }
    }
}

#endif
