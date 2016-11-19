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

#include "dec/alice_soft/afa_archive_decoder.h"
#include "algo/pack/zlib.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::alice_soft;

static std::unique_ptr<io::File> create_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const int version)
{
    off_t offset = 0;
    io::MemoryByteStream table_stream;
    for (const auto &file : expected_files)
    {
        table_stream.write("JUNK"_b);
        table_stream.write_le<u32>(file->path.str().size());
        table_stream.write(file->path.str());
        table_stream.write("JUNK"_b);
        table_stream.write("JUNK"_b);
        if (version == 1)
            table_stream.write("JUNK"_b);
        table_stream.write_le<u32>(offset);
        table_stream.write_le<u32>(file->stream.size());
        offset += file->stream.size();
    }

    const auto table_data_orig = table_stream.seek(0).read_to_eof();
    const auto table_data_comp = algo::pack::zlib_deflate(table_data_orig);

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("AFAH"_b);
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("AlicArch"_b);
    output_file->stream.write_le<u32>(version);
    output_file->stream.write("JUNK"_b);
    const auto data_offset_stub = output_file->stream.pos();
    output_file->stream.write("STUB"_b);

    output_file->stream.write("INFO"_b);
    output_file->stream.write_le<u32>(table_data_comp.size());
    output_file->stream.write_le<u32>(table_data_orig.size());
    output_file->stream.write_le<u32>(expected_files.size());
    output_file->stream.write(table_data_comp);
    const auto data_offset = output_file->stream.pos();

    for (const auto &file : expected_files)
        output_file->stream.write(file->stream);

    output_file->stream.seek(data_offset_stub).write_le<u32>(data_offset);
    return output_file;
}

TEST_CASE("Alice Soft AFA archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    std::unique_ptr<io::File> input_file;

    SECTION("Version 1")
    {
        input_file = create_file(expected_files, 1);
    }

    SECTION("Version 2")
    {
        input_file = create_file(expected_files, 2);
    }

    const auto decoder = AfaArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
