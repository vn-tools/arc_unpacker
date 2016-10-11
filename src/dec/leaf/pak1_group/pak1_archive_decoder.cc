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

#include "dec/leaf/pak1_group/pak1_archive_decoder.h"
#include <map>
#include "algo/locale.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/leaf/pak1_group/grp_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        bool compressed;
    };
}

// Modified LZSS routine
// - starting position at 0 rather than 0xFEE
// - optionally, additional byte for repetition count
// - dictionary writing in two passes
static bstr custom_lzss_decompress(
    const bstr &input, size_t output_size, const size_t dict_capacity)
{
    std::vector<u8> dict(dict_capacity);
    size_t dict_size = 0;
    size_t dict_pos = 0;

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    auto input_ptr = input.get<const u8>();
    auto input_end = input.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = *input_ptr++ | 0xFF00;

        if (control & 1)
        {
            dict[dict_pos++] = *output_ptr++ = *input_ptr++;
            dict_pos %= dict_capacity;
            if (dict_size < dict_capacity)
                dict_size++;
        }
        else
        {
            auto tmp = *reinterpret_cast<const u16*>(input_ptr);
            input_ptr += 2;

            auto look_behind_pos = tmp >> 4;
            auto repetitions = tmp & 0xF;
            if (repetitions == 0xF)
                repetitions += *input_ptr++;
            repetitions += 3;

            auto i = repetitions;
            while (i-- && output_ptr < output_end)
            {
                *output_ptr++ = dict[look_behind_pos++];
                look_behind_pos %= dict_size;
            }

            auto source = &output_ptr[-repetitions];
            while (source < output_ptr)
            {
                dict[dict_pos++] = *source++;
                dict_pos %= dict_capacity;
                if (dict_size < dict_capacity)
                    dict_size++;
            }
        }
    }

    return output;
}

Pak1ArchiveDecoder::Pak1ArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--pak-version"})
                ->set_value_name("NUMBER")
                ->set_description("File version (1 or 2)");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("pak-version"))
            {
                set_version(algo::from_string<int>(
                    arg_parser.get_switch("pak-version")));
            }
        });
}

void Pak1ArchiveDecoder::set_version(const int v)
{
    if (v != 1 && v != 2)
        throw err::UsageError("PAK version can be either '1' or '2'");
    version = v;
}

bool Pak1ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    auto meta = read_meta(dummy_logger, input_file);
    if (!meta->entries.size())
        return false;
    auto last_entry = static_cast<const CustomArchiveEntry*>(
        meta->entries[meta->entries.size() - 1].get());
    return last_entry->offset + last_entry->size == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> Pak1ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto file_count = input_file.stream.seek(0).read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(16)).str();
        entry->size = input_file.stream.read_le<u32>();
        entry->compressed = input_file.stream.read_le<u32>() > 0;
        entry->offset = input_file.stream.read_le<u32>();
        if (entry->size)
            meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak1ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    if (!version)
    {
        throw err::UsageError(
            "Please choose PAK version with --pak-version switch.");
    }

    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    bstr data;
    if (entry->compressed)
    {
        const auto size_comp = input_file.stream.read_le<u32>();
        const auto size_orig = input_file.stream.read_le<u32>();
        data = input_file.stream.read(size_comp - 8);
        data = custom_lzss_decompress(
            data, size_orig, version == 1 ? 0x1000 : 0x800);
    }
    else
    {
        data = input_file.stream.read(entry->size);
    }

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pak1ArchiveDecoder::get_linked_formats() const
{
    return {"leaf/grp"};
}

static auto _ = dec::register_decoder<Pak1ArchiveDecoder>("leaf/pak1");
