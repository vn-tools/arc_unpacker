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

#include "dec/ivory/mbl_archive_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/ivory/prs_image_decoder.h"
#include "dec/ivory/wady_audio_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::ivory;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bool encrypted;
        MblDecryptFunc decrypt;
    };
}

static bool verify_version(
    io::BaseByteStream &input_stream,
    const size_t file_count,
    const size_t name_size)
{
    try
    {
        input_stream.skip((file_count - 1) * (name_size + 8));
        input_stream.skip(name_size);
        const auto last_file_offset = input_stream.read_le<u32>();
        const auto last_file_size = input_stream.read_le<u32>();
        return last_file_offset + last_file_size == input_stream.size();
    }
    catch (...)
    {
        return false;
    }
}

static int detect_version(io::BaseByteStream &input_stream)
{
    input_stream.seek(0);
    const auto file_count = input_stream.read_le<u32>();
    if (verify_version(input_stream, file_count, 16))
        return 1;

    input_stream.seek(4);
    const auto name_size = input_stream.read_le<u32>();
    if (verify_version(input_stream, file_count, name_size))
        return 2;

    throw err::RecognitionError();
}

MblArchiveDecoder::MblArchiveDecoder()
{
    plugin_manager.add("noop", "Unencrypted games", [](bstr &) { });

    plugin_manager.add(
        "candy",
        "Candy Toys",
        [](bstr &data)
        {
            for (const auto i : algo::range(data.size()))
                data[i] = -data[i];
        });

    plugin_manager.add(
        "wanko",
        "Wanko to Kurasou",
        [](bstr &data)
        {
            static const bstr key =
                "\x82\xED\x82\xF1\x82\xB1\x88\xC3\x8D\x86\x89\xBB"_b;
            for (const auto i : algo::range(data.size()))
                data[i] ^= key[i % key.size()];
        });

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Specifies plugin for decoding dialog files."));
}

bool MblArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return detect_version(input_file.stream) > 0;
}

std::unique_ptr<dec::ArchiveMeta> MblArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    const auto version = detect_version(input_file.stream);
    meta->encrypted
        = input_file.path.name().find("mg_data") != std::string::npos;
    meta->decrypt = plugin_manager.is_set() ? plugin_manager.get() : nullptr;
    input_file.stream.seek(0);

    const auto file_count = input_file.stream.read_le<u32>();
    const auto name_size = version == 2
        ? input_file.stream.read_le<u32>()
        : 16;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(name_size)).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> MblArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);

    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    if (meta->encrypted)
    {
        if (!meta->decrypt)
        {
            throw err::UsageError(
                "File is encrypted, but plugin not set. "
                "Please supply one with --plugin switch.");
        }
        meta->decrypt(data);
    }

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> MblArchiveDecoder::get_linked_formats() const
{
    return {"ivory/prs", "ivory/wady"};
}

static auto _ = dec::register_decoder<MblArchiveDecoder>("ivory/mbl");
