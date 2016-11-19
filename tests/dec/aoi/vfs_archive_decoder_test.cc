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

#include "dec/aoi/vfs_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::aoi;

TEST_CASE("Aoi VFS archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    SECTION("Version 1.0")
    {
        io::File input_file;
        const auto entry_size = 0x13 + 8;
        const auto table_size = entry_size * expected_files.size();
        input_file.stream.write("VF\x00\x01"_b);
        input_file.stream.write_le<u16>(expected_files.size());
        input_file.stream.write_le<u16>(entry_size);
        input_file.stream.write_le<u32>(table_size);
        input_file.stream.write("JUNK"_b);

        size_t offset = input_file.stream.pos() + table_size;
        for (const auto &file : expected_files)
        {
            input_file.stream.write_zero_padded(file->path.str(), 0x13);
            input_file.stream.write_le<u32>(offset);
            input_file.stream.write_le<u32>(file->stream.size());
            offset += file->stream.size();
        }
        for (const auto &file : expected_files)
            input_file.stream.write(file->stream.seek(0).read_to_eof());

        const auto decoder = VfsArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }

    SECTION("Version 1.1")
    {
        io::File input_file;
        const auto entry_size = 0x13 + 8;
        const auto table_size = entry_size * expected_files.size();
        input_file.stream.write("VF\x01\x01"_b);
        input_file.stream.write_le<u16>(expected_files.size());
        input_file.stream.write_le<u16>(entry_size);
        input_file.stream.write_le<u32>(table_size);
        input_file.stream.write("JUNK"_b);

        size_t offset = input_file.stream.pos() + table_size;
        for (const auto &file : expected_files)
        {
            input_file.stream.write_zero_padded(file->path.str(), 0x13);
            input_file.stream.write_le<u32>(offset);
            input_file.stream.write_le<u32>(file->stream.size());
            offset += file->stream.size();
        }
        for (const auto &file : expected_files)
            input_file.stream.write(file->stream.seek(0).read_to_eof());

        const auto decoder = VfsArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }

    SECTION("Version 2.0")
    {
        io::File input_file;
        const auto entry_size = 4 + 6 + 8;
        const auto table_size = entry_size * expected_files.size();
        const auto names_offset = 16 + table_size + 8;
        auto names_size = 0;
        for (const auto &file : expected_files)
            names_size += (file->path.str().size() + 1) * 2;

        input_file.stream.write("VF\x00\x02"_b);
        input_file.stream.write_le<u16>(expected_files.size());
        input_file.stream.write_le<u16>(entry_size);
        input_file.stream.write_le<u32>(table_size);
        input_file.stream.write("JUNK"_b);
        size_t name_offset = 0;
        size_t data_offset = names_offset + names_size;
        for (const auto &file : expected_files)
        {
            input_file.stream.write_le<u32>(name_offset);
            input_file.stream.write("??????"_b);
            input_file.stream.write_le<u32>(data_offset);
            input_file.stream.write_le<u32>(file->stream.size());
            data_offset += file->stream.size();
            name_offset += file->path.str().size() + 1;
        }

        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        for (const auto &file : expected_files)
        {
            input_file.stream.write(algo::utf8_to_utf16(file->path.str()));
            input_file.stream.write("\x00\x00"_b);
        }

        for (const auto &file : expected_files)
            input_file.stream.write(file->stream.seek(0).read_to_eof());

        const auto decoder = VfsArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }
}
