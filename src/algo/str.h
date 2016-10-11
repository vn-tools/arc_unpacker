// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
