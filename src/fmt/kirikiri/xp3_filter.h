#pragma once

#include <functional>
#include <string>
#include "types.h"

namespace au {
namespace fmt {
namespace kirikiri {

    using Xp3FilterFunc = std::function<void(bstr &, u32)>;

    struct Xp3Filter final
    {
        Xp3Filter(const std::string &arc_path);
        std::string arc_path;
        Xp3FilterFunc decoder;
    };

} } }
