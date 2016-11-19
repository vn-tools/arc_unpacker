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

#include "dec/abogado/dsk_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::abogado;

static constexpr u32 mask = ((1 << 11) - 1);

static std::unique_ptr<io::File> get_dsk_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.DSK", ""_b);
    for (const auto &file : expected_files)
    {
        output_file->stream.write(file->stream.seek(0));
        while (output_file->stream.pos() & mask)
            output_file->stream.write<u8>('-');
    }
    return output_file;
}

static std::unique_ptr<io::File> get_pft_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.PFT", ""_b);
    output_file->stream.write("\x10\x00\x00\x08"_b);
    output_file->stream.write_le<u32>(expected_files.size());
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("JUNK"_b);

    auto current_offset = 0_z;
    for (const auto &file : expected_files)
    {
        auto size_padded = file->stream.size();
        while (size_padded & mask)
            size_padded++;

        output_file->stream.write_zero_padded(file->path.str(), 8);
        output_file->stream.write_le<u32>(current_offset >> 11);
        output_file->stream.write_le<u32>(file->stream.size());

        current_offset += size_padded;
    }

    return output_file;
}

TEST_CASE("Abogado DSK+PFT archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto input_file = get_dsk_file(expected_files);
    VirtualFileSystem::register_file(
        "test.PFT",
        [&]() { return get_pft_file(expected_files); });

    const auto decoder = DskArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
