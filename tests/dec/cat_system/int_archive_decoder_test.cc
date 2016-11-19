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

#include "dec/cat_system/int_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::cat_system;

static const std::string dir = "tests/dec/cat_system/files/int/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto decoder = IntArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("CatSystem INT archives", "[dec]")
{
    do_test(
        "ptcl.int",
        {
            tests::file_from_path(dir + "ptcl~.int/ase.kcs", "ase.kcs"),
            tests::file_from_path(dir + "ptcl~.int/ase2.kcs", "ase2.kcs"),
            tests::file_from_path(dir + "ptcl~.int/bhole.kcs", "bhole.kcs"),
            tests::file_from_path(dir + "ptcl~.int/bubble.kcs", "bubble.kcs"),
            tests::file_from_path(dir + "ptcl~.int/burst01.kcs", "burst01.kcs"),
            tests::file_from_path(dir + "ptcl~.int/burst02.kcs", "burst02.kcs"),
            tests::file_from_path(dir + "ptcl~.int/burst03.kcs", "burst03.kcs"),
            tests::file_from_path(dir + "ptcl~.int/flare.kcs", "flare.kcs"),
            tests::file_from_path(dir + "ptcl~.int/poison.kcs", "poison.kcs"),
            tests::file_from_path(dir + "ptcl~.int/rain.kcs", "rain.kcs"),
            tests::file_from_path(dir + "ptcl~.int/snow.kcs", "snow.kcs"),
            tests::file_from_path(dir + "ptcl~.int/spark.kcs", "spark.kcs"),
            tests::file_from_path(dir + "ptcl~.int/wind.kcs", "wind.kcs"),
        });
}
