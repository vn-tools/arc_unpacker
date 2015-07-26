#include <memory>
#include "fmt/kirikiri/xp3_filters/cxdec.h"
#include "fmt/kirikiri/xp3_filters/cxdec_mahoyoru.h"

using namespace au::fmt::kirikiri::xp3_filters;

CxdecSettings CxdecMahoYoru::get_settings() const
{
    CxdecSettings settings;
    settings.key1 = 0x22a;
    settings.key2 = 0x2a2;

    settings.key_derivation_order1[0] = 1;
    settings.key_derivation_order1[1] = 0;
    settings.key_derivation_order1[2] = 2;

    settings.key_derivation_order2[0] = 7;
    settings.key_derivation_order2[1] = 6;
    settings.key_derivation_order2[2] = 5;
    settings.key_derivation_order2[3] = 1;
    settings.key_derivation_order2[4] = 0;
    settings.key_derivation_order2[5] = 3;
    settings.key_derivation_order2[6] = 4;
    settings.key_derivation_order2[7] = 2;

    settings.key_derivation_order3[0] = 3;
    settings.key_derivation_order3[1] = 2;
    settings.key_derivation_order3[2] = 1;
    settings.key_derivation_order3[3] = 4;
    settings.key_derivation_order3[4] = 5;
    settings.key_derivation_order3[5] = 0;

    return settings;
}
