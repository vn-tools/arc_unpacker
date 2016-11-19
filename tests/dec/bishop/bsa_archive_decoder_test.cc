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

#include "dec/bishop/bsa_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::bishop;

static std::unique_ptr<io::File> get_bsa_file_v2(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.bsa", ""_b);
    output_file->stream.write("BSArc\x00\x00\x00"_b);
    output_file->stream.write_le<u16>(2);
    output_file->stream.write_le<u16>(expected_files.size());

    const auto table_offset_stub = output_file->stream.pos();
    output_file->stream.write("STUB"_b);

    const auto data_offset = output_file->stream.pos();
    for (const auto &file : expected_files)
        output_file->stream.write(file->stream.seek(0));

    const auto table_offset = output_file->stream.pos();
    auto current_offset = data_offset;
    for (const auto &file : expected_files)
    {
        output_file->stream.write_zero_padded(file->path.str(), 32);
        if (file->stream.size())
        {
            output_file->stream.write_le<u32>(current_offset);
            output_file->stream.write_le<u32>(file->stream.size());
        }
        current_offset += file->stream.size();
    }

    output_file->stream.seek(table_offset_stub).write_le<u32>(table_offset);
    return output_file;
}

static std::unique_ptr<io::File> get_bsa_file_v3(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.bsa", ""_b);
    output_file->stream.write("BSArc\x00\x00\x00"_b);
    output_file->stream.write_le<u16>(3);
    output_file->stream.write_le<u16>(expected_files.size());

    const auto table_offset_stub = output_file->stream.pos();
    output_file->stream.write("STUB"_b);

    const auto data_offset = output_file->stream.pos();
    for (const auto &file : expected_files)
        output_file->stream.write(file->stream.seek(0));

    const auto table_offset = output_file->stream.pos();
    auto current_offset = data_offset;
    auto file_name_offset = 0;
    for (const auto &file : expected_files)
    {
        output_file->stream.write_le<u32>(file_name_offset);
        output_file->stream.write_le<u32>(
            file->stream.size() ? current_offset : 0);
        output_file->stream.write_le<u32>(file->stream.size());
        current_offset += file->stream.size();
        file_name_offset += file->path.str().size() + 1;
    }
    for (const auto &file : expected_files)
    {
        output_file->stream.write(file->path.str());
        output_file->stream.write<u8>(0);
    }

    output_file->stream.seek(table_offset_stub).write_le<u32>(table_offset);
    return output_file;
}

TEST_CASE("Bishop BSA archives", "[dec]")
{
    const auto decoder = BsaArchiveDecoder();

    SECTION("Flat")
    {
        const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };
        std::unique_ptr<io::File> input_file;

        SECTION("V2")
        {
            input_file = get_bsa_file_v2(expected_files);
        }
        SECTION("V3")
        {
            input_file = get_bsa_file_v2(expected_files);
        }

        const auto actual_files = tests::unpack(decoder, *input_file);
        tests::compare_files(actual_files, expected_files, true);
    }

    SECTION("Hierarchical")
    {
        const std::vector<std::shared_ptr<io::File>> input_files =
        {
            tests::stub_file(">dir", ""_b),
            tests::stub_file(">sub", ""_b),
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("<", ""_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            tests::stub_file("<", ""_b),
            tests::stub_file(">other", ""_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            tests::stub_file("<", ""_b),
        };
        const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("dir/sub/123.txt", "1234567890"_b),
            tests::stub_file("dir/abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            tests::stub_file("other/abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };
        std::unique_ptr<io::File> input_file;

        SECTION("V2")
        {
            input_file = get_bsa_file_v2(input_files);
        }
        SECTION("V3")
        {
            input_file = get_bsa_file_v3(input_files);
        }

        const auto actual_files = tests::unpack(decoder, *input_file);
        tests::compare_files(actual_files, expected_files, true);
    }
}
