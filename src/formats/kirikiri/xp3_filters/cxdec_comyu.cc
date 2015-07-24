#include <memory>
#include "formats/kirikiri/xp3_filters/cxdec.h"
#include "formats/kirikiri/xp3_filters/cxdec_comyu.h"
using namespace Formats::Kirikiri::Xp3Filters;

CxdecSettings CxdecComyu::get_settings() const
{
    CxdecSettings settings;
    settings.key1 = 0x1a3;
    settings.key2 = 0x0b6;

    settings.key_derivation_order1[0] = 0;
    settings.key_derivation_order1[1] = 1;
    settings.key_derivation_order1[2] = 2;

    settings.key_derivation_order2[0] = 0;
    settings.key_derivation_order2[1] = 7;
    settings.key_derivation_order2[2] = 5;
    settings.key_derivation_order2[3] = 6;
    settings.key_derivation_order2[4] = 3;
    settings.key_derivation_order2[5] = 1;
    settings.key_derivation_order2[6] = 4;
    settings.key_derivation_order2[7] = 2;

    settings.key_derivation_order3[0] = 4;
    settings.key_derivation_order3[1] = 3;
    settings.key_derivation_order3[2] = 2;
    settings.key_derivation_order3[3] = 1;
    settings.key_derivation_order3[4] = 5;
    settings.key_derivation_order3[5] = 0;

    return settings;
}
