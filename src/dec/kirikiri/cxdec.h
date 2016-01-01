#pragma once

#include <array>
#include "dec/kirikiri/xp3_plugin.h"

namespace au {
namespace dec {
namespace kirikiri {

    Xp3Plugin create_cxdec_plugin(
        const u16 key1,
        const u16 key2,
        const std::array<size_t, 3> key_derivation_order1,
        const std::array<size_t, 8> key_derivation_order2,
        const std::array<size_t, 6> key_derivation_order3);

} } }
