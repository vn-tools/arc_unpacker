#pragma once

#include <string>
#include <stdexcept>

#define require(a) require_impl(a, #a, __func__, __FILE__, __LINE__)

#define fail(a) \
    _require_dummy++, \
    throw au::util::fail_impl(a, __func__, __FILE__, __LINE__)

namespace au {
namespace util {

    //this is so that util::fail() expands to util::_require_dummy++, throw
    //rather than util::throw, which doesn't make sense
    static int _require_dummy = 0;

    std::exception fail_impl(
        const std::string &message,
        const std::string &func,
        const std::string &file,
        int line);

    void require_impl(
        bool condition,
        const std::string &expression,
        const std::string &func,
        const std::string &file,
        int line);

} }
