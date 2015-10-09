#pragma once

#include <string>
#include <cstdarg>

namespace au {
namespace util {

    std::string format(std::string fmt, ...);
    std::string format(std::string fmt, std::va_list args);

} }
