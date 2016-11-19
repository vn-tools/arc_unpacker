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

#include "dec/nitroplus/npk2_archive_decoder.h"
#include <map>
#include "algo/crypt/aes.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::nitroplus;

namespace
{
    struct Segment final
    {
        size_t offset;
        bstr data_orig;
        bstr data_crypt;
        bstr data_comp;
    };
}

static bstr encrypt(const bstr &input, const bstr &iv, const bstr &key)
{
    return algo::crypt::aes256_encrypt_cbc(input, iv, key);
}

static std::unique_ptr<io::File> create_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bstr &iv,
    const bstr &key,
    const bool compressed,
    const size_t max_segment_size)
{
    std::map<io::path, std::vector<Segment>> segments;
    for (const auto &file : expected_files)
    {
        const auto content = file->stream.seek(0).read_to_eof();
        for (const auto i : algo::range(0, content.size(), max_segment_size))
        {
            Segment segment;
            segment.data_orig = content.substr(i, max_segment_size);
            segment.data_comp = compressed
                ? algo::pack::zlib_deflate(
                    segment.data_orig, algo::pack::ZlibKind::RawDeflate)
                : segment.data_orig;
            segment.data_crypt
                = segment.data_orig.size() < segment.data_comp.size()
                    ? encrypt(segment.data_orig, iv, key)
                    : encrypt(segment.data_comp, iv, key);
            segments[file->path].push_back(segment);
        }
    }

    auto table_size = 0;
    for (const auto &file : expected_files)
    {
        table_size += 1;
        table_size += 2;
        table_size += algo::utf8_to_sjis(file->path.str()).size();
        table_size += 36;
        table_size += 4;
        table_size += (8 + 4 + 4 + 4) * segments[file->path].size();
    }
    table_size += 0xF;
    table_size &= ~0xF;

    auto offset = 32 + table_size;
    for (const auto &file : expected_files)
    for (auto &segment : segments[file->path])
    {
        segment.offset = offset;
        offset += segment.data_crypt.size();
    }

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("NPK2"_b);
    output_file->stream.write("JUNK"_b);
    output_file->stream.write(iv);
    output_file->stream.write_le<u32>(expected_files.size());
    output_file->stream.write_le<u32>(table_size);

    io::MemoryByteStream table_stream;
    for (const auto &file : expected_files)
    {
        const auto name = algo::utf8_to_sjis(file->path.str());
        table_stream.write<u8>('?');
        table_stream.write_le<u16>(name.size());
        table_stream.write(name);
        table_stream.write_zero_padded("JUNK"_b, 36);
        const auto &file_segments = segments[file->path];
        table_stream.write_le<u32>(file_segments.size());
        for (const auto &segment : file_segments)
        {
            table_stream.write_le<u64>(segment.offset);
            table_stream.write_le<u32>(segment.data_crypt.size());
            table_stream.write_le<u32>(segment.data_comp.size());
            table_stream.write_le<u32>(segment.data_orig.size());
        }
    }

    auto table_data = table_stream.seek(0).read_to_eof();
    table_data = encrypt(table_data, iv, key);
    REQUIRE(table_data.size() == table_size);
    output_file->stream.write(table_data);

    for (const auto &file : expected_files)
    for (const auto &segment : segments[file->path])
    {
        REQUIRE(output_file->stream.pos() == segment.offset);
        output_file->stream.write(segment.data_crypt);
    }

    return output_file;
}

static void do_test(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bool compressed,
    const size_t chunk_size)
{
    const auto iv =
        "\xDD\x0D\x23\x7B\x29\xB9\x60\x66"
        "\x3E\x73\xC9\x71\x1D\x1C\x6C\x83"_b;
    const auto key =
        "\x96\x2C\x5F\x3A\x78\x9C\x84\x37"
        "\xB7\x12\x12\xA1\x15\xD6\xCA\x9F"
        "\x9A\xE3\xFD\x21\x0F\xF6\xAF\x70"
        "\xA8\xA8\xF8\xBB\xFE\x5E\x8A\xF5"_b;

    Npk2ArchiveDecoder decoder;
    decoder.plugin_manager.set("tokyo-necro");

    auto input_file = create_file(
        expected_files, iv, key, compressed, chunk_size);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Nitroplus NPK2 archives", "[dec]")
{
    SECTION("Uncompressed")
    {
        do_test(
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
                tests::stub_file("nice.txt", bstr(1024, 'a')),
            },
            false,
            1);
    }

    SECTION("Compressed")
    {
        do_test(
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
                tests::stub_file("nice.txt", bstr(1024, 'a')),
            },
            true,
            100);
    }

    SECTION("Compression skipped")
    {
        do_test(
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
                tests::stub_file("nice.txt", bstr(1024, 'a')),
            },
            true,
            1);
    }
}
