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

#include "dec/abstraction/wad_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::abstraction;

namespace
{
    struct DirEntry final
    {
        std::string name;
        u8 type;
    };

    struct Dir final
    {
        std::string name;
        std::vector<DirEntry> entries;
    };
}

static void write_string(
    io::BaseByteStream &output_stream, const bstr &input_string)
{
    const auto converted = algo::utf8_to_sjis(input_string);
    output_stream.write_le<u32>(converted.size());
    output_stream.write(converted);
}

static std::unique_ptr<io::File> get_wad_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const std::vector<Dir> &dirs)
{
    const auto extra_header = "herp derp"_b;

    auto output_file = std::make_unique<io::File>("test.wad", ""_b);
    output_file->stream.write("AGAR"_b);
    output_file->stream.write_le<u32>(1);
    output_file->stream.write_le<u32>(1);
    output_file->stream.write_le<u32>(extra_header.size());
    output_file->stream.write(extra_header);

    size_t current_offset = 0;
    output_file->stream.write_le<u32>(expected_files.size());
    for (const auto &file : expected_files)
    {
        write_string(output_file->stream, file->path.str());
        output_file->stream.write_le<u64>(file->stream.size());
        output_file->stream.write_le<u64>(current_offset);
        current_offset += file->stream.size();
    }

    output_file->stream.write_le<u32>(dirs.size());
    for (const auto &dir : dirs)
    {
        write_string(output_file->stream, dir.name);
        output_file->stream.write_le<u32>(dir.entries.size());
        for (const auto &dir_entry : dir.entries)
        {
            write_string(output_file->stream, dir_entry.name);
            output_file->stream.write<u8>(dir_entry.type);
        }
    }

    for (const auto &file : expected_files)
    {
        auto data = file->stream.seek(0).read_to_eof();
        output_file->stream.write(data);
    }

    return output_file;
}

TEST_CASE("Abstraction Games WAD archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("00000.dat", "1234567890"_b),
        tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    std::vector<Dir> dirs =
    {
        {
            "test",
            {
                {"test", 1},
                {"test2", 1},
            }
        },
        {
            "test2",
            {
                {"test", 1},
                {"test2", 1},
                {"test3", 1},
            }
        },
    };

    auto input_file = get_wad_file(expected_files, dirs);
    const auto decoder = WadArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
