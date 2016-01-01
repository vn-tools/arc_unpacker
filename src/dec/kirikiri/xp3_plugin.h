#pragma once

#include <functional>
#include "io/path.h"
#include "types.h"

namespace au {
namespace dec {
namespace kirikiri {

    using Xp3DecryptFunc = std::function<void(bstr &data, u32 key)>;

    struct Xp3Plugin final
    {
        std::function<Xp3DecryptFunc(const io::path &arc_path)>
            create_decrypt_func;
    };

} } }
