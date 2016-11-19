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

#include "dec/system_epsilon/packdat_archive_decoder.h"
#include "algo/binary.h"
#include "algo/ptr.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::system_epsilon;

static void encrypt(bstr &data)
{
    u32 key = data.size() >> 2;
    key = (key << ((key & 7) + 8)) ^ key;
    auto data_ptr = algo::make_ptr(data.get<u32>(), data.size() / 4);
    while (data_ptr.left())
    {
        const auto old_key = key;
        key = algo::rotl<u32>(key, *data_ptr % 24);
        *data_ptr++ ^= old_key;
    }
}

static void do_test(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bool encrypted)
{
    const auto decoder = PackdatArchiveDecoder();
    io::File input_file;
    input_file.stream.write("PACKDAT."_b);
    input_file.stream.write_le<u32>(expected_files.size());
    input_file.stream.write("JUNK"_b);
    auto offset = 16 + expected_files.size() * 48;
    for (const auto &file : expected_files)
    {
        input_file.stream.write_zero_padded(file->path.str(), 32);
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(encrypted ? 0x10000 : 0);
        input_file.stream.write_le<u32>(file->stream.size());
        input_file.stream.write_le<u32>(file->stream.size());
        offset += file->stream.size();
    }
    for (const auto &file : expected_files)
    {
        auto data = file->stream.seek(0).read_to_eof();
        if (encrypted)
            encrypt(data);
        input_file.stream.write(data);
    }
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("SYSTEM-ε PACKDAT archives", "[dec]")
{
    SECTION("Unencrypted")
    {
        do_test(
            {
                tests::stub_file("00000.dat", "1234567890"_b),
                tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
            },
            false);
    }

    SECTION("Encrypted")
    {
        do_test(
            {
                tests::stub_file("00000.dat", "1234567890"_b),
                tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
            },
            true);
    }
}
