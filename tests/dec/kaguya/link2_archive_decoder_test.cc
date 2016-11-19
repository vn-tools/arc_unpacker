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

#include "dec/kaguya/link2_archive_decoder.h"
#include "algo/binary.h"
#include "algo/pack/lzss.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

namespace
{
    class CustomLzssWriter final : public algo::pack::BaseLzssWriter
    {
    public:
        CustomLzssWriter(const size_t reserve_size);
        void write_literal(const u8 literal) override;
        void write_repetition(
            const size_t position_bits,
            const size_t position,
            const size_t size_bits,
            const size_t size) override;
        bstr retrieve() override;

    private:
        size_t control_count, control_pos;
        size_t repeat_count, repeat_pos;
        bstr output;
    };
}

CustomLzssWriter::CustomLzssWriter(const size_t reserve_size)
    : control_count(0), repeat_count(0)
{
    output.reserve(reserve_size);
}

void CustomLzssWriter::write_literal(const u8 literal)
{
    if (control_count == 0)
    {
        output += "\x00"_b;
        control_pos = output.size() - 1;
        control_count = 8;
    }
    output[control_pos] <<= 1;
    output[control_pos] |= 1;
    control_count--;
    output += literal;
}

void CustomLzssWriter::write_repetition(
    const size_t position_bits,
    const size_t position,
    const size_t size_bits,
    const size_t size)
{
    if (control_count == 0)
    {
        output += "\x00"_b;
        control_pos = output.size() - 1;
        control_count = 8;
    }
    output[control_pos] <<= 1;
    control_count--;
    output += static_cast<u8>(position);
    if (repeat_count == 0)
    {
        output += "\x00"_b;
        repeat_pos = output.size() - 1;
        repeat_count = 8;
    }
    output[repeat_pos] >>= 4;
    output[repeat_pos] |= size << 4;
    repeat_count -= 4;
}

bstr CustomLzssWriter::retrieve()
{
    if (control_count)
        output[control_pos] <<= control_count;
    if (repeat_count)
        output[repeat_pos] <<= repeat_count;
    return output;
}

static bstr compress(const bstr &input)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 8;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 0xEF;
    CustomLzssWriter writer(input.size());
    io::MemoryByteStream input_stream(input);
    return algo::pack::lzss_compress(input_stream, settings, writer);
}

TEST_CASE("Atelier Kaguya LINK2 archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const auto header_size = 8;
    auto table_size = 0u;
    for (const auto &file : expected_files)
        table_size += 2 + file->path.str().size() + 1 + 10;
    const auto data_offset = header_size + table_size;

    SECTION("Uncompressed")
    {
        io::File input_file;
        input_file.stream.write("LIN2"_b);
        input_file.stream.write_le<u32>(expected_files.size());
        auto offset = data_offset;
        for (const auto &file : expected_files)
        {
            input_file.stream.write_le<u16>(file->path.str().size() + 1);
            input_file.stream.write(algo::unxor(file->path.str(), 0xFF));
            input_file.stream.write<u8>(0xFF);
            input_file.stream.write_le<u32>(offset);
            input_file.stream.write_le<u32>(file->stream.size());
            input_file.stream.write_le<u16>(0);
            offset += file->stream.size();
        }
        REQUIRE(input_file.stream.pos() == data_offset);
        for (const auto &file : expected_files)
            input_file.stream.write(file->stream.seek(0).read_to_eof());

        const auto decoder = Link2ArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }

    SECTION("Compressed")
    {
        io::File input_file;
        input_file.stream.write("LIN2"_b);
        input_file.stream.write_le<u32>(expected_files.size());
        auto offset = data_offset;
        for (const auto &file : expected_files)
        {
            const auto data = compress(file->stream.seek(0).read_to_eof());
            input_file.stream.write_le<u16>(file->path.str().size() + 1);
            input_file.stream.write(algo::unxor(file->path.str(), 0xFF));
            input_file.stream.write<u8>(0xFF);
            input_file.stream.write_le<u32>(offset);
            input_file.stream.write_le<u32>(data.size() + 4);
            input_file.stream.write_le<u16>(1);
            offset += data.size() + 4;
        }
        REQUIRE(input_file.stream.pos() == data_offset);
        for (const auto &file : expected_files)
        {
            const auto data = compress(file->stream.seek(0).read_to_eof());
            input_file.stream.write_le<u32>(file->stream.size());
            input_file.stream.write(data);
        }

        const auto decoder = Link2ArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }
}
