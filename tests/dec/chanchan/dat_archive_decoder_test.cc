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

#include "dec/chanchan/dat_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::chanchan;

TEST_CASE("ChanChan DAT archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    auto input_file = std::make_unique<io::File>("test.dat", ""_b);

    SECTION("Variant 1")
    {
        input_file->stream.write_le<u32>(expected_files.size());
        for (const auto &file : expected_files)
        {
            input_file->stream.write_zero_padded(file->path.str(), 256);
            input_file->stream.write_le<u32>(file->stream.size());
            input_file->stream.write(file->stream.seek(0));
        }
    }

    SECTION("Variant 2")
    {
        const auto table_size = 8 + (256 + 8) * expected_files.size();
        input_file->stream.write_le<u32>(table_size);
        input_file->stream.write_le<u32>(expected_files.size());
        auto offset = table_size;
        for (const auto &file : expected_files)
        {
            input_file->stream.write_zero_padded(file->path.str(), 256);
            input_file->stream.write_le<u32>(file->stream.size());
            input_file->stream.write_le<u32>(offset);
            offset += file->stream.size();
        }
        for (const auto &file : expected_files)
            input_file->stream.write(file->stream.seek(0));
    }

    SECTION("Variant 3")
    {
        const auto table_size = 32 + (256 + 16) * expected_files.size();
        input_file->stream.write_le<u32>(expected_files.size());
        for (const auto i : algo::range(7))
            input_file->stream.write("JUNK"_b);
        auto entry_offset = table_size;
        for (const auto &file : expected_files)
        {
            const auto data_orig = file->stream.seek(0).read_to_eof();
            const auto data_comp = algo::pack::lzss_compress(data_orig);
            const auto entry_size = data_comp.size() + 36;
            input_file->stream.write_zero_padded(file->path.str(), 256);
            input_file->stream.write_le<u32>(entry_size);
            input_file->stream.write_le<u32>(entry_offset);
            input_file->stream.write("JUNKJUNK"_b);
            entry_offset += entry_size;
        }
        for (const auto &file : expected_files)
        {
            const auto data_orig = file->stream.seek(0).read_to_eof();
            const auto data_comp = algo::pack::lzss_compress(data_orig);
            input_file->stream.write("HERPDERP"_b);
            input_file->stream.write("JUNKJUNK"_b);
            input_file->stream.write_le<u32>(data_orig.size());
            input_file->stream.write_le<u32>(data_comp.size());
            input_file->stream.write("EVENMOREJUNK"_b);
            input_file->stream.write(data_comp);
        }
    }


    const auto decoder = DatArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
