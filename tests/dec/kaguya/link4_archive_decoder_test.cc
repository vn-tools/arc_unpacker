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

#include "dec/kaguya/link4_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::kaguya;

static std::unique_ptr<io::File> create_scr_file(const bstr &key)
{
    auto output_file = std::make_unique<io::File>("params.dat", ""_b);
    output_file->stream.write("[SCR-PARAMS]v05.2"_b);
    output_file->stream.write("TRASHTRASH"_b);
    output_file->stream.write<u8>(0);
    output_file->stream.write_le<u16>(0);
    output_file->stream.write_le<u16>(0);
    output_file->stream.write_le<u16>(0);
    output_file->stream.write<u8>('?');
    output_file->stream.write_le<u16>(0);
    output_file->stream.write_le<u16>(0);
    output_file->stream.write<u8>(0);
    output_file->stream.write("JUNKJUNKJUNKJUNK"_b);
    output_file->stream.write<u8>(0);
    output_file->stream.write<u8>(0);
    output_file->stream.write<u8>(0);
    output_file->stream.write_le<u32>(key.size());
    output_file->stream.write(key);
    return output_file;
}

TEST_CASE("Atelier Kaguya LINK4 archives", "[dec]")
{
    const auto arc_name = "tst"_b;

    SECTION("Unencrypted")
    {
        const std::vector<std::shared_ptr<io::File>> &expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            tests::stub_file("魔法.txt", "expecto patronum"_b),
        };

        io::File input_file;
        input_file.stream.write("LINK4"_b);
        input_file.stream.write(arc_name);
        input_file.stream.write("??"_b);
        for (const auto &file : expected_files)
        {
            const auto data = file->stream.seek(0).read_to_eof();
            const auto name = algo::utf8_to_sjis(file->path.str());
            input_file.stream.write_le<u32>(data.size() + 16 + name.size());
            input_file.stream.write<u8>(0);
            input_file.stream.write("????????"_b);
            input_file.stream.write<u8>(name.size());
            input_file.stream.write("??"_b);
            input_file.stream.write(name);
            input_file.stream.write(data);
        }
        input_file.stream.write_le<u32>(0);

        const auto decoder = Link4ArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }

    SECTION("Encrypted")
    {
        const std::vector<std::shared_ptr<io::File>> &expected_files =
        {
            tests::stub_file("encrypted.txt", "AP-0\0\0\0\0\0\0\0\0ABCDEF"_b),
        };

        const bstr key(240'000, '\xFF');

        VirtualFileSystem::register_file(
            "params.dat", [&]() { return create_scr_file(key); });

        io::File input_file;
        input_file.stream.write("LINK4"_b);
        input_file.stream.write(arc_name);
        input_file.stream.write("??"_b);
        for (const auto &file : expected_files)
        {
            auto data = file->stream.seek(0).read_to_eof();
            data = data.substr(0, 12) + algo::unxor(data.substr(12), key);
            const auto name = algo::utf8_to_sjis(file->path.str());
            input_file.stream.write_le<u32>(data.size() + 16 + name.size());
            input_file.stream.write_le<u16>(4);
            input_file.stream.write("???????"_b);
            input_file.stream.write<u8>(name.size());
            input_file.stream.write("??"_b);
            input_file.stream.write(name);
            input_file.stream.write(data);
        }
        input_file.stream.write_le<u32>(0);

        const auto decoder = Link4ArchiveDecoder();
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_files(actual_files, expected_files, true);
    }
}
