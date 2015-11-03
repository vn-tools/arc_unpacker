#pragma once

#include "io/io.h"

namespace au {
namespace util {
namespace pack {

    bstr zlib_inflate(io::IO &io);
    bstr zlib_inflate(const bstr &input);

} } }
