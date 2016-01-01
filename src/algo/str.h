#pragma once

#include "types.h"

namespace au {
namespace algo {

    std::string lower(const std::string &input);

    bstr reverse(const bstr &input);

    std::string reverse(const std::string &input);

    std::string hex(const bstr &input);

    std::string hex_verbose(const bstr &input, const size_t columns = 16);

    bstr unhex(const std::string &input);

    std::string trim_to_zero(const std::string &input);

    bstr trim_to_zero(const bstr &input);

    std::vector<std::string> split(
        const std::string &input,
        const char separator,
        const bool keep_separators);

    std::string replace_all(
        const std::string &input,
        const std::string &from,
        const std::string &to);

    template<typename T> T from_string(const std::string &input);

} }
