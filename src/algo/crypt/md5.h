#pragma once

#include <array>
#include <memory>
#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    bstr md5(const bstr &input);
    bstr md5(const bstr &input, const std::array<u32, 4> &custom_init);

} } }
