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

#include "dec/almond_collective/pac2_archive_decoder.h"
#include "algo/pack/zlib.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::almond_collective;

static std::unique_ptr<io::File> create_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bstr &key)
{
    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("PAC2"_b);
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("CCOD"_b);
    output_file->stream.write_le<u32>(key.size());
    output_file->stream.write(key);
    output_file->stream.write("FNUM"_b);
    output_file->stream.write_le<u32>(4);
    output_file->stream.write_le<u32>(expected_files.size());

    for (const auto &file : expected_files)
    {
        auto data = file->stream.seek(0).read_to_eof();
        const auto name = file->path.str();
        const auto entry_size = 16 + name.size() + data.size();
        output_file->stream.write("FILE"_b);
        output_file->stream.write_le<u32>(entry_size);
        output_file->stream.write("NAME"_b);
        output_file->stream.write_le<u32>(name.size());
        output_file->stream.write(name);
        output_file->stream.write("DATA"_b);
        output_file->stream.write_le<u32>(data.size());
        auto key_pos = output_file->stream.pos();
        for (const auto i : algo::range(data.size()))
            data[i] ^= key[(key_pos++) % key.size()];
        output_file->stream.write(data);
    }
    return output_file;
}

TEST_CASE("Almond Collective PAC2 archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    std::unique_ptr<io::File> input_file;

    input_file = create_file(expected_files, "herpderp"_b);

    const auto decoder = Pac2ArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
