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

#include "dec/qlie/pack_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/borland/tpf0_decoder.h"
#include "dec/microsoft/exe_archive_decoder.h"
#include "dec/qlie/mt.h"
#include "err.h"
#include "io/file_system.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::qlie;

static const bstr magic = "FilePackVer3.0\x00\x00"_b;
static const bstr compression_magic = "1PC\xFF"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr key1;
        bstr key2;
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bstr path_orig;
        bool encrypted;
        bool compressed;
        u32 seed;
    };
}

static size_t get_magic_start(const io::BaseByteStream &input_stream)
{
    return input_stream.size() - magic.size() - 8 - 4;
}

static u32 derive_seed(const bstr &input)
{
    u64 key = 0;
    u64 result = 0;
    for (const auto i : algo::range(input.size() >> 3))
    {
        key = algo::padw(key, 0x0307030703070307);
        result = algo::padw(result, input.get<u64>()[i] ^ key);
    }
    result ^= (result >> 32);
    return static_cast<u32>(result & 0xFFFFFFFF);
}

static void decrypt_file_name(bstr &file_name, const u32 key)
{
    const u8 x = ((key ^ 0x3E) + file_name.size()) & 0xFF;
    for (const auto i : algo::range(1, file_name.size() + 1))
        file_name.get<u8>()[i - 1] ^= ((i ^ x) & 0xFF) + i;
}

static void decrypt_file_data_basic(bstr &data, const u32 seed)
{
    u64 *current = data.get<u64>();
    const u64 *end = current + data.size() / 8;
    u64 key = 0xA73C5F9DA73C5F9D;
    u64 mutator = (seed + data.size()) ^ 0xFEC9753E;
    mutator = (mutator << 32) | mutator;

    while (current < end)
    {
        key = algo::padd(key, 0xCE24F523CE24F523);
        key ^= mutator;
        mutator = *current++ ^= key;
    }
}

static void decrypt_file_data_with_external_keys(
    bstr &data,
    const u32 seed,
    const bstr &file_name,
    const CustomArchiveMeta &meta)
{
    u32 mt_mutator = 0x85F532;
    u32 mt_seed = 0x33F641;

    for (const auto i : algo::range(file_name.size()))
    {
        mt_mutator += file_name.get<const u8>()[i] * static_cast<u8>(i);
        mt_seed ^= mt_mutator;
    }

    mt_seed += seed ^ (7 * (data.size() & 0xFFFFFF)
        + data.size()
        + mt_mutator
        + (mt_mutator ^ data.size() ^ 0x8F32DC));
    mt_seed = 9 * (mt_seed & 0xFFFFFF);

    if (!meta.key2.empty())
        mt_seed ^= 0x453A;

    CustomMersenneTwister mt(mt_seed);
    mt.xor_state(meta.key1);
    mt.xor_state(meta.key2);

    std::vector<u64> table(16);
    for (const auto i : algo::range(table.size()))
    {
        table[i]
            = mt.get_next_integer()
            | (static_cast<u64>(mt.get_next_integer()) << 32);
    }
    for (const auto i : algo::range(9))
        mt.get_next_integer();

    u64 mutator
        = mt.get_next_integer()
        | (static_cast<u64>(mt.get_next_integer()) << 32);

    auto table_index = mt.get_next_integer() % table.size();
    auto data_ptr = algo::make_ptr(data.get<u64>(), data.size() / 8);
    while (data_ptr.left())
    {
        mutator ^= table[table_index];
        mutator = algo::padd(mutator, table[table_index]);

        *data_ptr ^= mutator;

        mutator = algo::padb(mutator, *data_ptr);
        mutator ^= *data_ptr;
        mutator <<= 1;
        mutator &= 0xFFFFFFFEFFFFFFFE;
        mutator = algo::padw(mutator, *data_ptr);

        table_index++;
        table_index %= table.size();
        data_ptr++;
    }
}

static void decrypt_file_data(
    bstr &data,
    const u32 seed,
    const bstr &file_name,
    const CustomArchiveMeta &meta)
{
    if (meta.key1.empty() && meta.key2.empty())
        decrypt_file_data_basic(data, seed);
    else
        decrypt_file_data_with_external_keys(data, seed, file_name, meta);
}

static bstr decompress(const bstr &input, const size_t output_size)
{
    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);

    io::MemoryByteStream input_stream(input);

    if (input_stream.read(compression_magic.size()) != compression_magic)
    {
        throw err::CorruptDataError(
            "Unexpected magic in compressed file. "
            "Try with --fkey or --game-exe?");
    }

    const bool use_short_size = input_stream.read_le<u32>() > 0;
    const auto file_size = input_stream.read_le<u32>();
    if (file_size != output_size)
        throw err::BadDataSizeError();

    u8 dict1[256];
    u8 dict2[256];
    u8 dict3[256];
    while (input_stream.left())
    {
        for (const auto i : algo::range(256))
            dict1[i] = i;

        for (size_t d = 0; d < 256; )
        {
            u8 c = input_stream.read<u8>();
            if (c > 0x7F)
            {
                d += c - 0x7F;
                c = 0;
                if (d >= 256)
                    break;
            }

            for (const auto i : algo::range(c + 1))
            {
                dict1[d] = input_stream.read<u8>();
                if (dict1[d] != d)
                    dict2[d] = input_stream.read<u8>();
                d++;
            }
        }

        int bytes_left = use_short_size
            ? input_stream.read_le<u16>()
            : input_stream.read_le<u32>();

        u8 n = 0;
        while (output_ptr.left())
        {
            u8 d;
            if (n > 0)
            {
                d = dict3[--n];
            }
            else
            {
                if (bytes_left == 0)
                    break;
                bytes_left--;
                d = input_stream.read<u8>();
            }

            if (dict1[d] == d)
            {
                *output_ptr++ = d;
            }
            else
            {
                dict3[n++] = dict2[d];
                dict3[n++] = dict1[d];
            }
        }
    }
    return output;
}

static bstr get_fkey(const io::path &input_path)
{
    io::File file(input_path, io::FileMode::Read);
    return file.stream.seek(0).read_to_eof();
}

static bstr get_exe_key(const Logger &logger, const io::path &input_path)
{
    io::File exe_file(input_path, io::FileMode::Read);
    const auto exe_decoder = dec::microsoft::ExeArchiveDecoder();
    const auto tpf0_decoder = dec::borland::Tpf0Decoder();
    const auto exe_meta = exe_decoder.read_meta(logger, exe_file);

    std::unique_ptr<dec::ArchiveEntry> tform_entry;
    for (auto &entry : exe_meta->entries)
        if (entry->path.str().find("TFORM1") != std::string::npos)
            tform_entry = std::move(entry);
    if (!tform_entry)
        throw err::RecognitionError("Cannot find the key - missing TForm");

    const auto tform_file = exe_decoder.read_file(
        logger, exe_file, *exe_meta, *tform_entry);
    const auto tform_data = tform_file->stream.seek(0).read_to_eof();
    const auto tform = tpf0_decoder.decode(tform_data);

    std::unique_ptr<dec::borland::Tpf0Structure> ticon;
    for (auto &child : tform->children)
        if (child->name == "IconKeyImage")
            ticon = std::move(child);
    if (!ticon)
        throw err::RecognitionError("Cannot find the key - missing TIcon");

    const auto ticon_content = ticon->property<bstr>("Picture.Data");
    if (ticon_content.substr(0, 6) != "\x05TIcon"_b)
        throw err::RecognitionError("Malformed TIcon key");

    return ticon_content.substr(6, 256);
}

PackArchiveDecoder::PackArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_flag({"--no-external-keys"})
                ->set_description("Disables external archive keys");

            arg_parser.register_switch({"--fkey"})
                ->set_value_name("PATH")
                ->set_description("Selects path to fkey file");

            arg_parser.register_switch({"--game-exe"})
                ->set_value_name("PATH")
                ->set_description("Selects path to game executable");
        },
        [&](const ArgParser &arg_parser)
        {
            use_external_keys = !arg_parser.has_flag("no-external-keys");

            if (arg_parser.has_switch("fkey"))
                fkey_path = arg_parser.get_switch("fkey");

            if (arg_parser.has_switch("game-exe"))
                game_exe_path = arg_parser.get_switch("game-exe");
        });
}

bool PackArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(get_magic_start(input_file.stream));
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PackArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();

    if (use_external_keys)
    {
        if (!fkey_path.empty())
            meta->key1 = get_fkey(fkey_path);
        if (!game_exe_path.empty())
            meta->key2 = get_exe_key(logger, game_exe_path);

        if (meta->key1.empty() || meta->key2.empty())
        {
            const auto dir = input_file.path.parent().parent();
            logger.info("Searching for archive keys in %s...\n", dir.c_str());
            for (const auto &path : io::recursive_directory_range(dir))
            {
                if (!io::is_regular_file(path))
                    continue;
                if (path.has_extension("fkey") && meta->key1.empty())
                {
                    meta->key1 = get_fkey(path);
                    logger.info("Found fkey in %s\n", path.c_str());
                }
                if (path.has_extension("exe") && meta->key2.empty())
                {
                    try
                    {
                        meta->key2 = get_exe_key(logger, path);
                        logger.info("Found .exe key in %s\n", path.c_str());
                    }
                    catch (...)
                    {
                    }
                }
            }
        }
        if (meta->key1.empty())
            logger.info("fkey not found\n");
        if (meta->key2.empty())
            logger.info(".exe key not found\n");
    }

    input_file.stream.seek(get_magic_start(input_file.stream) + magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_offset = input_file.stream.read_le<u64>();
    const auto table_size = get_magic_start(input_file.stream) - table_offset;
    input_file.stream.seek(table_offset);
    io::MemoryByteStream table_stream(input_file.stream, table_size);

    u32 seed = 0;
    table_stream.seek(0);
    for (const auto i : algo::range(file_count))
    {
        size_t file_name_size = table_stream.read_le<u16>();
        table_stream.skip(file_name_size + 28);
    }
    table_stream.skip(28);
    table_stream.skip(table_stream.read_le<u32>());
    table_stream.skip(36);
    seed = derive_seed(table_stream.read(256)) & 0x0FFFFFFF;

    table_stream.seek(0);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();

        size_t name_size = table_stream.read_le<u16>();
        entry->path_orig = table_stream.read(name_size);
        decrypt_file_name(entry->path_orig, seed);
        entry->path = algo::sjis_to_utf8(entry->path_orig).str();

        entry->offset = table_stream.read_le<u64>();
        entry->size_comp = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        entry->compressed = table_stream.read_le<u32>() > 0;
        entry->encrypted = table_stream.read_le<u32>() > 0;

        entry->seed = seed;
        table_stream.skip(4);

        meta->entries.push_back(std::move(entry));
    }

    if (use_external_keys)
    {
        for (const auto &entry : meta->entries)
        {
            if (entry->path.name().find("pack_keyfile") == std::string::npos)
                continue;
            try
            {
                auto file = read_file(logger, input_file, *meta, *entry);
                meta->key1 = file->stream.seek(0).read_to_eof();
            }
            catch (const std::exception &e)
            {
                logger.err(
                    "Error updating key: " + std::string(e.what()) + "\n");
            }
        }
    }

    return std::move(meta);
}

std::unique_ptr<io::File> PackArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    auto data = input_file.stream
        .seek(entry->offset)
        .read(entry->size_comp);

    if (entry->encrypted)
        decrypt_file_data(data, entry->seed, entry->path_orig, *meta);

    if (entry->compressed)
        data = decompress(data, entry->size_orig);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> PackArchiveDecoder::get_linked_formats() const
{
    return {"qlie/abmp7", "qlie/abmp10", "qlie/dpng"};
}

static auto _ = dec::register_decoder<PackArchiveDecoder>("qlie/pack");
