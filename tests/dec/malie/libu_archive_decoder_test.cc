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

#include "dec/malie/libu_archive_decoder.h"
#include "algo/crypt/camellia.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/malie/common/lib_plugins.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::malie;

static void encrypt(
    const std::vector<u32> &key,
    const bstr &input,
    io::BaseByteStream &output_stream)
{
    algo::crypt::Camellia camellia(key);
    const auto offset_start = output_stream.pos() & ~0xF;
    const auto aligned_size = (input.size() + 0xF) & ~0xF;
    const auto block_count = aligned_size / 0x10;
    io::MemoryByteStream input_stream;
    input_stream.write(bstr((-input.size()) & 0xF, 0xFF));
    input_stream.write(input);
    input_stream.seek(0);
    for (const auto i : algo::range(block_count))
    {
        u32 input_block[4];
        u32 output_block[4] = {1,2,3,4};
        for (const auto j : algo::range(4))
            input_block[j] = input_stream.read_be<u32>();
        camellia.encrypt_block_128(
            offset_start + i * 0x10,
            input_block,
            output_block);
        for (const auto j : algo::range(4))
            output_stream.write_le<u32>(output_block[j]);
    }
}

TEST_CASE("Malie Libu archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    SECTION("Unencrypted")
    {
        io::File input_file;
        LibuArchiveDecoder decoder;
        // do not set "noop" plugin - unencrypted files should get recognized
        // automatically, otherwise nested extraction will not work!

        input_file.stream.write("LIBU"_b);
        input_file.stream.write_le<u32>(0x10000);
        input_file.stream.write_le<u32>(expected_files.size());
        input_file.stream.write("JUNK"_b);
        auto offset = 16 + expected_files.size() * 80;
        for (const auto &file : expected_files)
        {
            input_file.stream.write_zero_padded(
                algo::utf8_to_utf16(file->path.str()), 68);
            input_file.stream.write_le<u32>(file->stream.size());
            input_file.stream.write_le<u32>(offset);
            input_file.stream.write("JUNK"_b);
            offset += file->stream.size();
        }
        for (const auto &file : expected_files)
            input_file.stream.write(file->stream.seek(0).read_to_eof());

        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }

    SECTION("Encrypted")
    {
        // dies irae key
        static const std::vector<u32> key = {
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x16C976B6, 0x6CEC462F, 0xBA30F99A, 0x6E6CDE71,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0xBB5B3676, 0x2317DD18, 0x7CCD3736, 0x6F388B64,
            0x9B3B118B, 0xEE8C3E66, 0x9B9B379C, 0x45B25DAD,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x88C5F746, 0x1F334DCD, 0x00000000, 0x00000000,
            0xFBA30F99, 0xA6E6CDE7, 0x116C976B, 0x66CEC462,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x9B9B379C, 0x45B25DAD, 0x9B3B118B, 0xEE8C3E66,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x6F388B64, 0xBB5B3676, 0x2317DD18, 0x7CCD3736,
        };

        io::File input_file;
        LibuArchiveDecoder decoder;
        decoder.plugin_manager.set("dies-irae");

        io::MemoryByteStream header_stream;
        header_stream.write("LIBU"_b);
        header_stream.write_le<u32>(0x10000);
        header_stream.write_le<u32>(expected_files.size());
        header_stream.write("JUNK"_b);
        encrypt(key, header_stream.seek(0).read_to_eof(), input_file.stream);

        io::MemoryByteStream table_stream;
        auto offset = ((16 + expected_files.size() * 80) + 0xF) & ~0xF;
        for (const auto &file : expected_files)
        {
            table_stream.write_zero_padded(
                algo::utf8_to_utf16(file->path.str()), 68);
            table_stream.write_le<u32>(file->stream.size());
            table_stream.write_le<u32>(offset + ((-file->stream.size()) & 0xF));
            table_stream.write("JUNK"_b);
            offset += (file->stream.size() + 0xF) & ~0xF;
        }
        encrypt(key, table_stream.seek(0).read_to_eof(), input_file.stream);

        for (const auto &file : expected_files)
            encrypt(key, file->stream.seek(0).read_to_eof(), input_file.stream);

        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }
}
