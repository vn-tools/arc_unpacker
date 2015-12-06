#pragma once

#include "types.h"

namespace au {
namespace util {
namespace algo {

    std::string lower(const std::string &input);

    std::string reverse(const std::string &input);

    std::string unhex(const std::string &input);

    std::string trim_to_zero(const std::string &input);

    bstr trim_to_zero(const bstr &input);

    std::string replace_all(
        const std::string &input,
        const std::string &from,
        const std::string &to);

    template<typename T> T from_string(const std::string &input);

} } }
