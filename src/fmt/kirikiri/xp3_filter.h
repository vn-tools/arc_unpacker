#pragma once

#include <functional>
#include "io/path.h"
#include "types.h"

namespace au {
namespace fmt {
namespace kirikiri {

    using Xp3FilterFunc = std::function<void(bstr &, u32)>;

    struct Xp3Filter final
    {
        Xp3Filter(const io::path &arc_path);
        io::path arc_path;
        Xp3FilterFunc decoder;
    };

} } }
