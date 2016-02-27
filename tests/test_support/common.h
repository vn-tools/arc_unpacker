#pragma once

#include "io/path.h"
#include "types.h"

namespace au {
namespace tests {

    void compare_paths(const io::path &actual, const io::path &expected);

    void compare_binary(const bstr &actual, const bstr &expected);

} }
