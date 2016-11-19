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

#include "test_support/common.h"
#include "algo/str.h"
#include "test_support/catch.h"

using namespace au;

void au::tests::compare_paths(
    const io::path &actual, const io::path &expected)
{
    INFO(actual.str() << " != " << expected.str());
    REQUIRE(actual == expected);
}

void au::tests::compare_binary(const bstr &actual, const bstr &expected)
{
    const auto max_size = 10000;
    auto actual_dump = algo::hex(actual);
    auto expected_dump = algo::hex(expected);
    if (actual_dump.size() > max_size)
        actual_dump = actual_dump.substr(0, max_size) + "(...)";
    if (expected_dump.size() > max_size)
        expected_dump = expected_dump.substr(0, max_size) + "(...)";
    INFO(actual_dump << " != " << expected_dump);
    REQUIRE(actual == expected);
}
