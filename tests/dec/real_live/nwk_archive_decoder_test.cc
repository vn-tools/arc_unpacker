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

#include "dec/real_live/nwk_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::real_live;

TEST_CASE("RealLive NWK archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("sample00000.nwa", "1234567890"_b),
        tests::stub_file("sample00001.nwa", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    io::File input_file("test.nwk", ""_b);
    input_file.stream.write_le<u32>(expected_files.size());
    auto offset = 4 + 12 * expected_files.size();
    for (const auto i : algo::range(expected_files.size()))
    {
        const auto &expected_file = expected_files[i];
        input_file.stream.write_le<u32>(expected_file->stream.size());
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(i);
        offset += expected_file->stream.size();
    }
    for (auto &expected_file : expected_files)
        input_file.stream.write(expected_file->stream.seek(0).read_to_eof());

    const auto decoder = NwkArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
