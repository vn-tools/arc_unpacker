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

#include "dec/kaguya/link_archive_decoder.h"
#include "algo/binary.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya LINK archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const auto header_size = 12;
    auto names_size = 0u;
    for (const auto &file : expected_files)
        names_size += file->path.str().size() + 1;
    const auto table_size = expected_files.size() * 8;
    const auto data_offset = header_size + names_size + table_size;

    io::File input_file;
    input_file.stream.write("LINK"_b);
    input_file.stream.write_le<u32>(expected_files.size());
    input_file.stream.write_le<u32>(names_size);
    for (const auto &file : expected_files)
    {
        input_file.stream.write(file->path.str());
        input_file.stream.write<u8>(0);
    }
    auto offset = data_offset;
    for (const auto &file : expected_files)
    {
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(file->stream.size());
        offset += file->stream.size();
    }
    for (const auto &file : expected_files)
        input_file.stream.write(file->stream.seek(0).read_to_eof());

    const auto decoder = LinkArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
