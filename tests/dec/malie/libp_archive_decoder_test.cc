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

#include "dec/malie/libp_archive_decoder.h"
#include <queue>
#include "algo/crypt/camellia.h"
#include "algo/range.h"
#include "dec/malie/common/lib_plugins.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::malie;

namespace
{
    struct BaseEntry
    {
        BaseEntry(const std::string &name) : name(name) {}
        virtual ~BaseEntry() {}

        std::string name;
        std::vector<std::shared_ptr<BaseEntry>> children;
    };

    struct DirEntry final : BaseEntry
    {
        DirEntry(const std::string &name) : BaseEntry(name) {}
    };

    struct FileEntry final : BaseEntry
    {
        FileEntry(const std::string &name, const bstr &content)
            : BaseEntry(name), content(content)
        {
        }

        bstr content;
    };
}

static bstr encrypt(const std::vector<u32> &key, const bstr &input)
{
    algo::crypt::Camellia camellia(key);
    io::MemoryByteStream output_stream;
    io::MemoryByteStream input_stream(input);
    input_stream.resize((input.size() + 0xF) & ~0xF);
    for (const auto i : algo::range(input.size() / 0x10))
    {
        u32 input_block[4];
        u32 output_block[4] = {1,2,3,4};
        for (const auto j : algo::range(4))
            input_block[j] = input_stream.read_be<u32>();
        camellia.encrypt_block_128(i * 0x10, input_block, output_block);
        for (const auto j : algo::range(4))
            output_stream.write_le<u32>(output_block[j]);
    }
    return output_stream.seek(0).read_to_eof();
}

static std::unique_ptr<io::File> pack(
    const std::shared_ptr<DirEntry> root_entry)
{
    io::MemoryByteStream table_stream;
    io::MemoryByteStream content_stream;
    std::vector<size_t> offsets;

    auto entry_count = 0u;
    auto entry_index = 1u;
    auto file_offset = 0u;
    std::queue<std::shared_ptr<BaseEntry>> queue;
    queue.push(root_entry);
    while (!queue.empty())
    {
        const auto &entry = queue.front();
        queue.pop();
        entry_count++;

        const auto dir_entry = dynamic_cast<const DirEntry*>(entry.get());
        const auto file_entry = dynamic_cast<const FileEntry*>(entry.get());
        table_stream.write_zero_padded(entry->name, 20);
        if (dir_entry)
        {
            table_stream.write_le<u32>(0);
            table_stream.write_le<u32>(entry_index);
            table_stream.write_le<u32>(entry->children.size());
            entry_index += entry->children.size();
        }
        else if (file_entry)
        {
            const auto content = file_entry->content;
            table_stream.write_le<u32>(0x10000);
            table_stream.write_le<u32>(offsets.size());
            table_stream.write_le<u32>(content.size());
            offsets.push_back(file_offset >> 10);
            content_stream.write(content);
            const auto stupid_align = (1 << 10) - 1;
            while (content_stream.pos() & stupid_align)
                content_stream.write<u8>('-');
            file_offset += (content.size() + stupid_align) & ~stupid_align;
        }
        else
            throw std::logic_error("Can't tell the nature of the entry");
        for (auto child : entry->children)
            queue.push(child);
    }

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("LIBP"_b);
    output_file->stream.write_le<u32>(entry_count);
    output_file->stream.write_le<u32>(offsets.size());
    output_file->stream.write("JUNK"_b);
    output_file->stream.write(table_stream.seek(0));
    for (const auto offset : offsets)
        output_file->stream.write_le<u32>(offset);
    while (output_file->stream.pos() & 0xFFF)
        output_file->stream.write<u8>('-');
    output_file->stream.write(content_stream.seek(0));
    return output_file;
}

TEST_CASE("Malie LIBP archives", "[dec]")
{
    auto tree = std::make_shared<DirEntry>("");
    tree->children.push_back(std::make_shared<DirEntry>("first"));
    tree->children.push_back(std::make_shared<DirEntry>("second"));
    tree->children[0]->children.push_back(
        std::make_shared<FileEntry>("1.txt", "1 blabla"_b)),
    tree->children[1]->children.push_back(
        std::make_shared<FileEntry>("2.txt", "2 herp derp"_b)),
    tree->children[1]->children.push_back(
        std::make_shared<DirEntry>("nested"));
    tree->children[1]->children.push_back(
        std::make_shared<FileEntry>("3.txt", "3 more content"_b)),
    tree->children[1]->children[1]->children.push_back(
        std::make_shared<FileEntry>("4.txt", "4 interesting"_b));

    const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("first/1.txt", "1 blabla"_b),
            tests::stub_file("second/2.txt", "2 herp derp"_b),
            tests::stub_file("second/nested/4.txt", "4 interesting"_b),
            tests::stub_file("second/3.txt", "3 more content"_b),
        };

    SECTION("Unencrypted")
    {
        // do not set "noop" plugin - unencrypted files should get recognized
        // automatically, otherwise nested extraction will not work!
        LibpArchiveDecoder decoder;

        auto input_file = pack(tree);
        const auto actual_files = tests::unpack(decoder, *input_file);
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

        LibpArchiveDecoder decoder;
        decoder.plugin_manager.set("dies-irae");

        auto input_file = pack(tree);
        input_file->stream.seek(0).write(
            encrypt(key, input_file->stream.seek(0).read_to_eof()));

        const auto actual_files = tests::unpack(decoder, *input_file);
        tests::compare_files(actual_files, expected_files, true);
    }
}
