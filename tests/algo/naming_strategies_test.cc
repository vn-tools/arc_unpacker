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

#include "algo/naming_strategies.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo;

static void do_test(
    const NamingStrategy strategy,
    const io::path &parent_path,
    const io::path &child_path,
    const io::path &expected_path)
{
    const auto actual_path = algo::apply_naming_strategy(
        strategy, parent_path, child_path);
    tests::compare_paths(actual_path, expected_path);
}

TEST_CASE("File naming strategies", "[fmt_core]")
{
    SECTION("Root")
    {
        const auto s = NamingStrategy::Root;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "file");
        do_test(s, "test/",      "file", "file");
        do_test(s, "test/nest",  "file", "file");
        do_test(s, "test/nest/", "file", "file");
        do_test(s, "test/nest",  "a/b", "a/b");
    }

    SECTION("Sibling")
    {
        const auto s = NamingStrategy::Sibling;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "file");
        do_test(s, "test/",      "file", "test/file");
        do_test(s, "test/nest",  "file", "test/file");
        do_test(s, "test/nest/", "file", "test/nest/file");
        do_test(s, "test/nest",  "a/b", "test/a/b");
    }

    SECTION("Flat sibling")
    {
        const auto s = NamingStrategy::FlatSibling;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "file");
        do_test(s, "test/",      "file", "test/file");
        do_test(s, "test/nest",  "file", "test/file");
        do_test(s, "test/nest/", "file", "test/nest/file");
        do_test(s, "test/nest",  "a/b", "test/b");
    }

    SECTION("Child")
    {
        const auto s = NamingStrategy::Child;
        do_test(s, "",           "file", "file");
        do_test(s, "test",       "file", "test/file");
        do_test(s, "test/",      "file", "test/file");
        do_test(s, "test/nest",  "file", "test/nest/file");
        do_test(s, "test/nest/", "file", "test/nest/file");
        do_test(s, "test/nest",  "a/b", "test/nest/a/b");
    }
}
