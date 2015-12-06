#pragma once

#include <cstdarg>
#include <string>

namespace au {
namespace algo {

    std::string format(const std::string fmt, ...);
    std::string format(const std::string fmt, std::va_list args);

} }
