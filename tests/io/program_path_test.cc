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

#include "io/program_path.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Getting program path", "[io]")
{
    const auto program_path = io::get_program_path();
    REQUIRE(program_path.str().find("run_tests") != std::string::npos);
}

TEST_CASE("Getting assets directory path", "[io]")
{
    const auto assets_dir_path = io::get_assets_dir_path();
    REQUIRE(assets_dir_path.str().find("etc") != std::string::npos);
}
