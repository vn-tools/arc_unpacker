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

#include "dec/triangle/med_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::triangle;

static std::unique_ptr<io::File> get_med_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto name_size = 123;
    auto output_file = std::make_unique<io::File>("test.med", ""_b);
    output_file->stream.write("MD"_b);
    output_file->stream.write("??"_b);
    output_file->stream.write_le<u16>(name_size + 8);
    output_file->stream.write_le<u16>(expected_files.size());
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("JUNK"_b);

    size_t current_offset
        = output_file->stream.pos() + (name_size + 8) * expected_files.size();
    for (const auto &file : expected_files)
    {
        output_file->stream.write_zero_padded(file->path.str(), name_size);
        output_file->stream.write_le<u32>(file->stream.size());
        output_file->stream.write_le<u32>(current_offset);
        current_offset += file->stream.size();
    }

    for (const auto &file : expected_files)
        output_file->stream.write(file->stream.seek(0));

    return output_file;
}

TEST_CASE("Triangle MED archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto input_file = get_med_file(expected_files);
    const auto decoder = MedArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
