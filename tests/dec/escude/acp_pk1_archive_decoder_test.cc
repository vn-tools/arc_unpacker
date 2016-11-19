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

#include "dec/escude/acp_pk1_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::escude;

static void do_test(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bstr &magic)
{
    const auto decoder = AcpPk1ArchiveDecoder();
    io::File input_file;
    input_file.stream.write(magic);
    input_file.stream.write_le<u32>(expected_files.size());
    auto offset = input_file.stream.pos() + expected_files.size() * (32 + 8);
    for (const auto &file : expected_files)
    {
        input_file.stream.write_zero_padded(file->path.str(), 32);
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(file->stream.size());
        offset += file->stream.size();
    }
    for (const auto &file : expected_files)
        input_file.stream.write(file->stream.seek(0).read_to_eof());
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Escude ACP-PK1 archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("00000.dat", "1234567890"_b),
        tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    do_test(expected_files, "ACP_PK.1"_b);
    do_test(expected_files, "ACPXPK01"_b);
}
