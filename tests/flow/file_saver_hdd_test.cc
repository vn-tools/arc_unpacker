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

#include "flow/file_saver_hdd.h"
#include "io/file_system.h"
#include "test_support/catch.h"

using namespace au;

static void do_test(const io::path &path)
{
    const flow::FileSaverHdd file_saver(".", true);
    const auto file = std::make_shared<io::File>(path.str(), "test"_b);

    file_saver.save(file);

    REQUIRE(io::exists(path));
    {
        io::FileByteStream file_stream(path, io::FileMode::Read);
        REQUIRE(file_stream.size() == 4);
        REQUIRE(file_stream.read_to_eof() == "test"_b);
    }
    io::remove(path);
}

static void do_test_overwriting(
    const flow::IFileSaver &file_saver1,
    const flow::IFileSaver &file_saver2,
    const bool renamed_file_exists)
{
    io::path path = "test.txt";
    io::path path2 = "test(1).txt";
    const auto file = std::make_shared<io::File>(path.str(), ""_b);

    try
    {
        REQUIRE(!io::exists(path));
        REQUIRE(!io::exists(path2));
        file_saver1.save(file);
        file_saver2.save(file);
        REQUIRE(io::exists(path));
        REQUIRE(io::exists(path2) == renamed_file_exists);
        if (io::exists(path)) io::remove(path);
        if (io::exists(path2)) io::remove(path2);
    }
    catch(...)
    {
        if (io::exists(path)) io::remove(path);
        if (io::exists(path2)) io::remove(path2);
        throw;
    }
}

TEST_CASE("FileSaver", "[core]")
{
    SECTION("Unicode file names")
    {
        do_test("test.out");
        do_test(u8"ąćę.out");
        do_test(u8"不用意な変換.out");
    }

    SECTION("Two file savers overwrite the same file")
    {
        const flow::FileSaverHdd file_saver1(".", true);
        const flow::FileSaverHdd file_saver2(".", true);
        do_test_overwriting(file_saver1, file_saver2, false);
    }

    SECTION("Two file savers don't overwrite the same file")
    {
        const flow::FileSaverHdd file_saver1(".", false);
        const flow::FileSaverHdd file_saver2(".", false);
        do_test_overwriting(file_saver1, file_saver2, true);
    }

    SECTION("One file saver never overwrites the same file")
    {
        // even if we pass overwrite=true, files within the same archive with
        // the same name are too valuable to be ovewritten silently
        const flow::FileSaverHdd file_saver(".", true);
        do_test_overwriting(file_saver, file_saver, true);
    }
}
