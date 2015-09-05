#pragma once

#include <array>
#include "fmt/kirikiri/xp3_filter.h"

namespace au {
namespace fmt {
namespace kirikiri {

    Xp3FilterFunc create_cxdec_filter(
        const std::string &arc_path,
        u16 key1,
        u16 key2,
        const std::array<size_t, 3> key_derivation_order1,
        const std::array<size_t, 8> key_derivation_order2,
        const std::array<size_t, 6> key_derivation_order3);

} } }
