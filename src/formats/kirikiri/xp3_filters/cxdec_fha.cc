#include <memory>
#include "formats/kirikiri/xp3_filters/cxdec.h"
#include "formats/kirikiri/xp3_filters/cxdec_fha.h"

using namespace au::fmt::kirikiri::xp3_filters;

CxdecSettings CxdecFha::get_settings() const
{
    CxdecSettings settings;
    settings.key1 = 0x143;
    settings.key2 = 0x787;

    settings.key_derivation_order1[0] = 0;
    settings.key_derivation_order1[1] = 1;
    settings.key_derivation_order1[2] = 2;

    settings.key_derivation_order2[0] = 0;
    settings.key_derivation_order2[1] = 1;
    settings.key_derivation_order2[2] = 2;
    settings.key_derivation_order2[3] = 3;
    settings.key_derivation_order2[4] = 4;
    settings.key_derivation_order2[5] = 5;
    settings.key_derivation_order2[6] = 6;
    settings.key_derivation_order2[7] = 7;

    settings.key_derivation_order3[0] = 0;
    settings.key_derivation_order3[1] = 1;
    settings.key_derivation_order3[2] = 2;
    settings.key_derivation_order3[3] = 3;
    settings.key_derivation_order3[4] = 4;
    settings.key_derivation_order3[5] = 5;

    return settings;
}
