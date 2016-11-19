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

#include "dec/will/pnap_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::will;

static std::unique_ptr<io::File> get_pnap_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.pna", ""_b);
    output_file->stream.write("PNAP"_b);
    for (const auto i : algo::range(3))
        output_file->stream.write("JUNK"_b);
    output_file->stream.write_le<u32>(expected_files.size());
    for (const auto &file : expected_files)
    {
        for (const auto i : algo::range(9))
            output_file->stream.write("JUNK"_b);
        output_file->stream.write_le<u32>(file->stream.size());
    }
    for (const auto &file : expected_files)
        output_file->stream.write(file->stream.seek(0));
    return output_file;
}

TEST_CASE("Will Co. PNAP archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto input_file = get_pnap_file(expected_files);
    const auto decoder = PnapArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, false);
}
