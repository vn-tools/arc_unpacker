#pragma once

#include <string>
#include <cstdarg>

namespace au {
namespace util {

    std::string format(const std::string &fmt, ...);
    std::string format(const std::string &fmt, std::va_list args);

} }
