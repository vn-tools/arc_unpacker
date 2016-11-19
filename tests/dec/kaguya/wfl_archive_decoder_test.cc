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

#include "dec/kaguya/wfl_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

static bstr compress(const bstr &input)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 12;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_compress(input, settings);
}

TEST_CASE("Atelier Kaguya WFL archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        tests::stub_file("魔法.txt", "expecto patronum"_b),
    };

    io::File input_file;
    input_file.stream.write("WFL1"_b);

    SECTION("Uncompressed")
    {
        for (const auto &file : expected_files)
        {
            const auto name = algo::unxor(
                algo::utf8_to_sjis(file->path.str()) + "\x00"_b, 0xFF);
            input_file.stream.write_le<u32>(name.size());
            input_file.stream.write(name);
            input_file.stream.write_le<u16>(0);
            input_file.stream.write_le<u32>(file->stream.size());
            input_file.stream.write(file->stream.seek(0).read_to_eof());
        }
    }

    SECTION("Compressed")
    {
        for (const auto &file : expected_files)
        {
            const auto name = algo::unxor(
                algo::utf8_to_sjis(file->path.str()) + "\x00"_b, 0xFF);
            input_file.stream.write_le<u32>(name.size());
            input_file.stream.write(name);
            const auto data_orig = file->stream.seek(0).read_to_eof();
            const auto data_comp = compress(data_orig);
            input_file.stream.write_le<u16>(1);
            input_file.stream.write_le<u32>(data_comp.size());
            input_file.stream.write_le<u32>(data_orig.size());
            input_file.stream.write(data_comp);
        }
    }

    const auto decoder = WflArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
