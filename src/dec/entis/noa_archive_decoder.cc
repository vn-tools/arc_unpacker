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

#include "dec/entis/noa_archive_decoder.h"
#include "algo/range.h"
#include "dec/entis/common/bshf_decoder.h"
#include "dec/entis/common/erisan_decoder.h"
#include "dec/entis/common/sections.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x04\x00\x02\x00\x00\x00\x00"_b;
static const bstr magic3 = "ERISA-Archive file"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::string key;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        u32 encryption;
        bstr extra;
    };
}

static std::unique_ptr<CustomArchiveMeta> read_meta(
    io::BaseByteStream &input_stream, const io::path root = "")
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    common::SectionReader section_reader(input_stream);
    for (const auto &section : section_reader.get_sections("DirEntry"))
    {
        input_stream.seek(section.data_offset);
        const auto entry_count = input_stream.read_le<u32>();
        for (const auto i : algo::range(entry_count))
        {
            auto entry = std::make_unique<CustomArchiveEntry>();
            entry->size = input_stream.read_le<u64>();
            const auto flags = input_stream.read_le<u32>();
            entry->encryption = input_stream.read_le<u32>();
            entry->offset = section.base_offset + input_stream.read_le<u64>();
            input_stream.skip(8);

            const auto extra_size = input_stream.read_le<u32>();
            if (flags & 0x70)
                entry->extra = input_stream.read(extra_size);

            const auto file_name_size = input_stream.read_le<u32>();
            entry->path = input_stream.read_to_zero(file_name_size).str();
            if (!root.str().empty())
                entry->path = root / entry->path;

            if (flags == 0x10)
            {
                const auto sub_meta = read_meta(input_stream, entry->path);
                input_stream.peek(entry->offset, [&]()
                {
                    for (auto &sub_entry : sub_meta->entries)
                        meta->entries.push_back(std::move(sub_entry));
                });
            }
            else if (flags == 0x20 || flags == 0x40)
            {
            }
            else
            {
                meta->entries.push_back(std::move(entry));
            }
        }
    }
    return meta;
}

bool NoaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic1.size()) == magic1
        && input_file.stream.read(magic2.size()) == magic2
        && input_file.stream.read(magic3.size()) == magic3;
}

NoaArchiveDecoder::NoaArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--noa-key"})
                ->set_value_name("KEY")
                ->set_description("Decryption key (same for all files)");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("noa-key"))
                key = arg_parser.get_switch("noa-key");
        });
}

std::unique_ptr<dec::ArchiveMeta> NoaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x40);
    auto meta = ::read_meta(input_file.stream);
    meta->key = key;
    return std::move(meta);
}

std::unique_ptr<io::File> NoaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    input_file.stream.seek(entry->offset);

    const auto entry_magic = input_file.stream.read(8);
    if (entry_magic != "filedata"_b)
        throw err::CorruptDataError("Expected 'filedata' magic.");

    const auto total_size = input_file.stream.read_le<u64>();
    auto data = input_file.stream.read(total_size);
    if (entry->encryption)
    {
        const auto expected_checksum = data.substr(-4);
        if (entry->encryption & 0x40000000)
        {
            if (meta->key.empty())
            {
                throw err::UsageError(
                    "File is encrypted, but key not set. "
                    "Please supply one with --noa-key switch.");
            }
            common::BshfDecoder decoder(meta->key);
            decoder.set_input(data);
            decoder.reset();
            decoder.decode(data.get<u8>(), entry->size);
        }

        if (entry->encryption & 0x80000010)
        {
            bstr output(entry->size);
            common::ErisaNDecoder decoder;
            decoder.set_input(data);
            decoder.reset();
            decoder.decode(output.get<u8>(), entry->size);
            data = output;
        }

        if (entry->encryption & ~(0x80000010 | 0x40000000))
        {
            logger.warn(
                "%s: unknown encryption scheme (%08x)\n",
                entry->path.c_str(),
                entry->encryption);
        }
        else
        {
            std::array<u8, 4> checksum = {0, 0, 0, 0};
            for (const auto i : algo::range(entry->size))
                checksum[i % checksum.size()] ^= data[i];
            for (const auto i : algo::range(4))
            {
                if (checksum[i] != expected_checksum[i])
                {
                    throw err::CorruptDataError(
                        "Checksum mismatch - wrong key?");
                }
            }
        }
    }

    const auto actual_data = data.substr(0, entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, actual_data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> NoaArchiveDecoder::get_linked_formats() const
{
    return {"entis/noa", "entis/mio", "entis/eri"};
}

static auto _ = dec::register_decoder<NoaArchiveDecoder>("entis/noa");
