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

#include "dec/twilight_frontier/tfpk_archive_decoder.h"
#include <map>
#include "algo/crypt/rsa.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/file_system.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::twilight_frontier;

static const bstr magic = "TFPK"_b;

namespace
{
    enum class TfpkVersion : u8
    {
        Th135,
        Th145,
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        TfpkVersion version;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        bstr key;
    };

    struct DirEntry final
    {
        u32 initial_hash;
        size_t file_count;
    };

    using HashLookupMap = std::map<u32, std::string>;

    class RsaReader final
    {
    public:
        RsaReader(io::BaseByteStream &input_stream);
        ~RsaReader();
        std::unique_ptr<io::MemoryByteStream> read_block();
        size_t pos() const;

    private:
        std::unique_ptr<io::MemoryByteStream>
            decrypt(std::basic_string<u8> input);

        io::BaseByteStream &input_stream;
        std::unique_ptr<algo::crypt::Rsa> rsa;
    };
}

static const std::vector<algo::crypt::RsaKey> rsa_keys({
    // TH13.5 Japanese version
    {
        {
            0xC7, 0x9A, 0x9E, 0x9B, 0xFB, 0xC2, 0x0C, 0xB0,
            0xC3, 0xE7, 0xAE, 0x27, 0x49, 0x67, 0x62, 0x8A,
            0x78, 0xBB, 0xD1, 0x2C, 0xB2, 0x4D, 0xF4, 0x87,
            0xC7, 0x09, 0x35, 0xF7, 0x01, 0xF8, 0x2E, 0xE5,
            0x49, 0x3B, 0x83, 0x6B, 0x84, 0x26, 0xAA, 0x42,
            0x9A, 0xE1, 0xCC, 0xEE, 0x08, 0xA2, 0x15, 0x1C,
            0x42, 0xE7, 0x48, 0xB1, 0x9C, 0xCE, 0x7A, 0xD9,
            0x40, 0x1A, 0x4D, 0xD4, 0x36, 0x37, 0x5C, 0x89
        },
        65537,
    },

    // TH13.5 English patch
    {
        {
            0xFF, 0x65, 0x72, 0x74, 0x61, 0x69, 0x52, 0x20,
            0x2D, 0x2D, 0x20, 0x69, 0x6F, 0x6B, 0x6E, 0x61,
            0x6C, 0x46, 0x20, 0x73, 0x73, 0x65, 0x6C, 0x42,
            0x20, 0x64, 0x6F, 0x47, 0x20, 0x79, 0x61, 0x4D,
            0x08, 0x8B, 0xF4, 0x75, 0x5D, 0x78, 0xB1, 0xC8,
            0x93, 0x7F, 0x40, 0xEA, 0x34, 0xA5, 0x85, 0xC1,
            0x1B, 0x8D, 0x63, 0x17, 0x75, 0x98, 0x2D, 0xA8,
            0x17, 0x45, 0x31, 0x31, 0x51, 0x4F, 0x6E, 0x8D
        },
        65537,
    },

    // TH14.5 Japanese version
    {
        {
            0xC6, 0x43, 0xE0, 0x9D, 0x35, 0x5E, 0x98, 0x1D,
            0xBE, 0x63, 0x6D, 0x3A, 0x5F, 0x84, 0x0F, 0x49,
            0xB8, 0xE8, 0x53, 0xF5, 0x42, 0x06, 0x37, 0x3B,
            0x36, 0x25, 0xCB, 0x65, 0xCE, 0xDD, 0x68, 0x8C,
            0xF7, 0x5D, 0x72, 0x0A, 0xC0, 0x47, 0xBD, 0xFA,
            0x3B, 0x10, 0x4C, 0xD2, 0x2C, 0xFE, 0x72, 0x03,
            0x10, 0x4D, 0xD8, 0x85, 0x15, 0x35, 0x55, 0xA3,
            0x5A, 0xAF, 0xC3, 0x4A, 0x3B, 0xF3, 0xE2, 0x37,
        },
        65537,
    },
});

RsaReader::RsaReader(io::BaseByteStream &input_stream)
    : input_stream(input_stream)
{
    // test chunk = one block with dir count
    bstr test_chunk;
    input_stream.peek(
        input_stream.pos(),
        [&]() { test_chunk = input_stream.read(0x40); });

    for (const auto &rsa_key : rsa_keys)
    {
        algo::crypt::Rsa tester(rsa_key);
        try
        {
            tester.decrypt(test_chunk);
            rsa = std::make_unique<algo::crypt::Rsa>(rsa_key);
            return;
        }
        catch (...)
        {
        }
    }

    // no encryption - TH14.5 English patch.
    // First 4 bytes are integer meaning dir count, so the rest should be zero.
    if (test_chunk.substr(4, 12) == "\0\0\0\0\0\0\0\0\0\0\0\0"_b)
        return;

    throw err::NotSupportedError("Unknown public key");
}

RsaReader::~RsaReader()
{
}

size_t RsaReader::pos() const
{
    return input_stream.pos();
}

std::unique_ptr<io::MemoryByteStream> RsaReader::read_block()
{
    const auto block = rsa
        ? rsa->decrypt(input_stream.read(0x40)).substr(0, 0x20)
        : input_stream.read(0x40).substr(0, 0x20);
    return std::make_unique<io::MemoryByteStream>(block);
}

static bstr read_file_content(
    io::File &input_file,
    const CustomArchiveMeta &meta,
    const CustomArchiveEntry &entry,
    size_t max_size)
{
    input_file.stream.seek(entry.offset);
    auto data = input_file.stream.read(std::min<size_t>(max_size, entry.size));
    const auto key_size = entry.key.size();
    if (meta.version == TfpkVersion::Th135)
    {
        for (const auto i : algo::range(data.size()))
            data[i] ^= entry.key[i % key_size];
    }
    else
    {
        auto *key = entry.key.get<const u8>();
        auto *buf = data.get<u8>();
        u8 aux[4];
        for (const auto i : algo::range(4))
            aux[i] = key[i];
        for (const auto i : algo::range(data.size()))
        {
            const auto tmp = buf[i];
            buf[i] = tmp ^ key[i % key_size] ^ aux[i & 3];
            aux[i & 3] = tmp;
        }
    }
    return data;
}

static u32 neg32(u32 x)
{
    return static_cast<u32>(-static_cast<s32>(x));
}

static std::string lower_ascii_only(std::string name_utf8)
{
    // while SJIS can use ASCII for encoding multibyte characters,
    // UTF-8 uses the codes 0..127 only for the ASCII characters.
    for (const auto i : algo::range(name_utf8.size()))
        if (name_utf8[i] >= 'A' && name_utf8[i] <= 'Z')
            name_utf8[i] += 'a' - 'A';
    return name_utf8;
}

static std::string replace_slash_with_backslash(std::string s)
{
    std::string s_copy = s;
    std::replace(s_copy.begin(), s_copy.end(), '/', '\\');
    return s_copy;
}

static u32 get_file_name_hash(
    const std::string name,
    TfpkVersion version,
    const u32 initial_hash = 0x811C9DC5)
{
    std::string name_processed = algo::utf8_to_sjis(
        replace_slash_with_backslash(lower_ascii_only(name))).str();

    if (version == TfpkVersion::Th135)
    {
        u32 result = initial_hash;
        for (const auto i : algo::range(name_processed.size()))
        {
            result *= 0x1000193;
            result ^= name_processed[i];
        }
        return result;
    }
    else
    {
        u32 result = initial_hash;
        for (const auto i : algo::range(name_processed.size()))
        {
            result ^= name_processed[i];
            result *= 0x1000193;
        }
        return neg32(result);
    }
}

static std::string get_unknown_name(
    int index, u32 hash, const std::string &ext = ".dat")
{
    if (index)
        return algo::format("unk-%05d-%08x%s", index, hash, ext.c_str());
    return algo::format("unk-%08x%s", hash, ext.c_str());
}

static std::string get_dir_name(
    const DirEntry &dir_entry, const HashLookupMap user_fn_map)
{
    const auto it = user_fn_map.find(dir_entry.initial_hash);
    if (it != user_fn_map.end())
        return it->second;
    return get_unknown_name(0, dir_entry.initial_hash, "");
}

static std::vector<DirEntry> read_dir_entries(RsaReader &reader)
{
    std::vector<DirEntry> dirs;
    const auto dir_count = reader.read_block()->read_le<u32>();
    for (const auto i : algo::range(dir_count))
    {
        auto tmp_stream = reader.read_block();
        DirEntry entry;
        entry.initial_hash = tmp_stream->read_le<u32>();
        entry.file_count = tmp_stream->read_le<u32>();
        dirs.push_back(entry);
    }
    return dirs;
}

static HashLookupMap read_fn_map(
    RsaReader &reader,
    const std::vector<DirEntry> &dir_entries,
    const HashLookupMap &user_fn_map,
    TfpkVersion version)
{
    HashLookupMap fn_map;

    auto tmp_stream = reader.read_block();
    const auto table_size_comp = tmp_stream->read_le<u32>();
    const auto table_size_orig = tmp_stream->read_le<u32>();
    const auto block_count = tmp_stream->read_le<u32>();

    tmp_stream = std::make_unique<io::MemoryByteStream>();
    for (const auto i : algo::range(block_count))
        tmp_stream->write(reader.read_block()->read_to_eof());

    tmp_stream->seek(0);
    tmp_stream = std::make_unique<io::MemoryByteStream>(
        algo::pack::zlib_inflate(tmp_stream->read(table_size_comp)));

    for (const auto &dir_entry : dir_entries)
    {
        auto dn = get_dir_name(dir_entry, user_fn_map);
        if (dn.size() > 0 && dn[dn.size() - 1] != '/')
            dn += "/";

        for (const auto j : algo::range(dir_entry.file_count))
        {
            // TH145 English patch contains invalid directory names
            try
            {
                const auto fn
                    = algo::sjis_to_utf8(tmp_stream->read_to_zero()).str();
                const auto hash = get_file_name_hash(
                    fn, version, dir_entry.initial_hash);
                fn_map[hash] = dn + fn;
            }
            catch (...)
            {
            }
        }
    }
    return fn_map;
}

TfpkArchiveDecoder::TfpkArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--file-names"})
                ->set_value_name("PATH")
                ->set_description(
                    "Specifies path to file containing list of game's file "
                    "names. Used to get proper file names and to find palettes "
                    "for sprites.");
        },
        [&](const ArgParser &arg_parser)
        {
            const auto path = arg_parser.get_switch("file-names");
            if (path != "")
            {
                io::FileByteStream stream(path, io::FileMode::Read);
                std::string line;
                while ((line = stream.read_line().str()) != "")
                    fn_set.insert(line);
            }
        });
}

bool TfpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    const auto version = input_file.stream.read<u8>();
    return version == 0 || version == 1;
}

std::unique_ptr<dec::ArchiveMeta> TfpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->version = input_file.stream.read<u8>() == 0
        ? TfpkVersion::Th135
        : TfpkVersion::Th145;

    HashLookupMap user_fn_map;
    for (const auto &fn : fn_set)
        user_fn_map[get_file_name_hash(fn, meta->version)] = fn;

    RsaReader reader(input_file.stream);
    HashLookupMap fn_map;

    // TH135 contains file hashes, TH145 contains garbage
    const auto dir_entries = read_dir_entries(reader);
    if (dir_entries.size() > 0)
        fn_map = read_fn_map(reader, dir_entries, user_fn_map, meta->version);

    for (const auto &it : user_fn_map)
        fn_map[it.first] = it.second;

    const auto file_count = reader.read_block()->read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto b1 = reader.read_block();
        const auto b2 = reader.read_block();
        const auto b3 = reader.read_block();
        if (meta->version == TfpkVersion::Th135)
        {
            entry->size = b1->read_le<u32>();
            entry->offset = b1->read_le<u32>();

            const auto fn_hash = b2->read_le<u32>();
            const auto it = fn_map.find(fn_hash);
            entry->path = it == fn_map.end()
                ? get_unknown_name(i, fn_hash)
                : it->second;

            entry->key = b3->read(16);
        }
        else
        {
            entry->size   = b1->read_le<u32>() ^ b3->read_le<u32>();
            entry->offset = b1->read_le<u32>() ^ b3->read_le<u32>();

            const auto fn_hash = b2->read_le<u32>() ^ b3->read_le<u32>();
            const auto unk = b2->read_le<u32>() ^ b3->read_le<u32>();
            const auto it = fn_map.find(fn_hash);
            entry->path = it == fn_map.end()
                ? get_unknown_name(i + 1, fn_hash)
                : it->second;

            b3->seek(0);
            io::MemoryByteStream key_stream;
            for (const auto j : algo::range(4))
                key_stream.write_le<u32>(neg32(b3->read_le<u32>()));

            key_stream.seek(0);
            entry->key = key_stream.read_to_eof();
        }
        meta->entries.push_back(std::move(entry));
    }

    auto table_end = reader.pos();
    for (auto &entry : meta->entries)
        static_cast<CustomArchiveEntry*>(entry.get())->offset += table_end;

    return std::move(meta);
}

std::unique_ptr<io::File> TfpkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    const auto data = read_file_content(input_file, *meta, *entry, entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> TfpkArchiveDecoder::get_linked_formats() const
{
    return
    {
        "twilight-frontier/tfcs",
        "twilight-frontier/tfwa",
        "twilight-frontier/tfbm",
        "microsoft/dds"
    };
}

static auto _ = dec::register_decoder<TfpkArchiveDecoder>(
    "twilight-frontier/tfpk");
