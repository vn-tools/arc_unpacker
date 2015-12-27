#pragma once

#include "io/path.h"
#include "types.h"

namespace au {
namespace fmt {

    enum class NamingStrategy : u8
    {
        Child = 0,
        Root = 1,
        Sibling = 2,
        FlatSibling = 3,
    };

    io::path decorate_path(
        const NamingStrategy strategy,
        const io::path &parent_name,
        const io::path &current_name);

} }
