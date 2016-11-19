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

#include "dec/kaguya/link6_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya LINK6 archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const auto arc_name = "test"_b;

    io::File input_file;
    input_file.stream.write("LINK6"_b);
    input_file.stream.write("??"_b);
    input_file.stream.write<u8>(arc_name.size());
    input_file.stream.write(arc_name);
    for (const auto &file : expected_files)
    {
        const auto data = file->stream.seek(0).read_to_eof();
        const auto name = algo::utf8_to_utf16(file->path.str());
        input_file.stream.write_le<u32>(data.size() + 15 + name.size());
        input_file.stream.write_le<u16>(0);
        input_file.stream.write("???????");
        input_file.stream.write_le<u16>(name.size());
        input_file.stream.write(name);
        input_file.stream.write(data);
    }
    input_file.stream.write_le<u32>(0);

    const auto decoder = Link6ArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
