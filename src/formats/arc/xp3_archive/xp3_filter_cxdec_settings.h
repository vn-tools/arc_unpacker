#ifndef FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_CXDEC_PLUGIN
#define FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_CXDEC_PLUGIN
#include <array>

class Xp3FilterCxdecSettings
{
public:
    uint16_t key1;
    uint16_t key2;
    std::array<size_t, 3> key_derivation_order1;
    std::array<size_t, 8> key_derivation_order2;
    std::array<size_t, 6> key_derivation_order3;
    const char *encryption_block;
};

#endif
