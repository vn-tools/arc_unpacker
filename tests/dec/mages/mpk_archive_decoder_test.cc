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

#include "dec/mages/mpk_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::mages;

static void write_toc_entry(
    io::BaseByteStream &output_stream,
    const u32 id,
    const u64 offset,
    const u64 size,
    const std::string &path)
{
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(id);
    output_stream.write_le<u64>(offset);
    output_stream.write_le<u64>(size);
    output_stream.write_le<u64>(size);
    output_stream.write_zero_padded(path, 224);
}

TEST_CASE("MAGES. MPK archives archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("WIN\\test1.txt", "hello"_b),
        tests::stub_file("test2.txt", "world"_b),
    };

    auto output_file = std::make_unique<io::File>("mini.mpk", ""_b);
    output_file->stream.write("MPK\0"_b);
    output_file->stream.write_le<u16>(0);
    output_file->stream.write_le<u16>(2);
    // file count may go over actual file count (but never outside of TOC)
    output_file->stream.write_le<u32>(expected_files.size() + 2);
    output_file->stream.write(bstr(52));

    for (size_t i = 0; i < expected_files.size(); i++)
    {
        if (expected_files[i]->path.str().empty())
            continue;
        // note: assumes TOC and every test file are <= 2KB each
        // IDs needn't be sequential, hence the random numbers
        write_toc_entry(
            output_file->stream,
            std::rand(),
            (i + 1) * 2048,
             expected_files[i]->stream.size(),
             expected_files[i]->path.str());
    }
    for (size_t i = 0; i < expected_files.size(); i++)
    {
        output_file->stream.write(
            bstr(((i + 1) * 2048) - output_file->stream.pos()));
        auto data = expected_files[i]->stream.seek(0).read_to_eof();
        output_file->stream.write(data);
    }

    const auto decoder = MpkArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *output_file);
    tests::compare_files(actual_files, expected_files, true);
}
