// TFPK archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - [Tasofro & Team Shanghai Alice] [130526] TH13.5 - Hopeless Masquerade
// - [Tasofro & Team Shanghai Alice] [150510] TH14.5 - Urban Legend in Limbo

#include <boost/filesystem.hpp>
#include <map>
#include <set>
#include "err.h"
#include "fmt/microsoft/dds_converter.h"
#include "fmt/touhou/tfbm_converter.h"
#include "fmt/touhou/tfcs_converter.h"
#include "fmt/touhou/tfpk_archive.h"
#include "fmt/touhou/tfwa_converter.h"
#include "io/buffered_io.h"
#include "util/crypt/rsa.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    class RsaReader
    {
    public:
        RsaReader(io::IO &io);
        ~RsaReader();
        std::unique_ptr<io::BufferedIO> read_block();
        size_t tell() const;
    private:
        std::unique_ptr<io::BufferedIO> decrypt(std::basic_string<u8> input);
        io::IO &io;
        std::unique_ptr<util::crypt::Rsa> rsa;
    };

    enum class TfpkVersion : u8
    {
        Th135,
        Th145,
    };

    struct TableEntry
    {
        size_t size;
        size_t offset;
        std::string name;
        bstr key;
    };

    struct DirEntry
    {
        u32 initial_hash;
        size_t file_count;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
    using DirTable = std::vector<std::unique_ptr<DirEntry>>;
    using HashLookupMap = std::map<u32, std::string>;
}

static const bstr magic = "TFPK"_b;

static const std::vector<util::crypt::RsaKey> rsa_keys({
    //TH13.5 Japanese version
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

    //TH13.5 English patch
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

    //TH14.5 Japanese version
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

RsaReader::RsaReader(io::IO &io) : io(io)
{
    bstr test_chunk;
    io.peek(io.tell(), [&]() { test_chunk = io.read(0x40); });

    for (auto &rsa_key : rsa_keys)
    {
        util::crypt::Rsa tester(rsa_key);
        try
        {
            tester.decrypt(test_chunk);
            rsa.reset(new util::crypt::Rsa(rsa_key));
            return;
        }
        catch (...)
        {
        }
    }
    throw err::NotSupportedError("Unknown public key");
}

RsaReader::~RsaReader()
{
}

size_t RsaReader::tell() const
{
    return io.tell();
}

std::unique_ptr<io::BufferedIO> RsaReader::read_block()
{
    auto block = rsa->decrypt(io.read(0x40)).substr(0, 0x20);
    return std::unique_ptr<io::BufferedIO>(new io::BufferedIO(block));
}

static u32 neg32(u32 x)
{
    return static_cast<u32>(-static_cast<i32>(x));
}

static std::string lower_ascii_only(std::string name_utf8)
{
    //while SJIS can use ASCII for encoding multibyte characters,
    //UTF-8 uses the codes 0–127 only for the ASCII characters.
    for (auto i : util::range(name_utf8.size()))
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
    u32 initial_hash = 0x811C9DC5)
{
    std::string name_processed = util::utf8_to_sjis(
        replace_slash_with_backslash(lower_ascii_only(name))).str();

    if (version == TfpkVersion::Th135)
    {
        u32 result = initial_hash;
        for (auto i : util::range(name_processed.size()))
        {
            result *= 0x1000193;
            result ^= name_processed[i];
        }
        return result;
    }
    else
    {
        u32 result = initial_hash;
        for (auto i : util::range(name_processed.size()))
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
        return util::format("unk-%05d-%08x%s", index, hash, ext.c_str());
    return util::format("unk-%08x%s", hash, ext.c_str());
}

static std::string get_dir_name(DirEntry &dir_entry, HashLookupMap user_fn_map)
{
    auto it = user_fn_map.find(dir_entry.initial_hash);
    if (it != user_fn_map.end())
        return it->second;
    return get_unknown_name(-1, dir_entry.initial_hash, "");
}

static DirTable read_dir_entries(RsaReader &reader)
{
    auto tmp_io = reader.read_block();
    std::vector<std::unique_ptr<DirEntry>> dir_entries;
    size_t dir_count = tmp_io->read_u32_le();
    for (auto i : util::range(dir_count))
    {
        tmp_io = reader.read_block();
        std::unique_ptr<DirEntry> entry(new DirEntry);
        entry->initial_hash = tmp_io->read_u32_le();
        entry->file_count = tmp_io->read_u32_le();
        dir_entries.push_back(std::move(entry));
    }
    return dir_entries;
}

static HashLookupMap read_fn_map(
    RsaReader &reader,
    DirTable &dir_entries,
    HashLookupMap &user_fn_map,
    TfpkVersion version)
{
    HashLookupMap fn_map;

    auto tmp_io = reader.read_block();
    size_t table_size_compressed = tmp_io->read_u32_le();
    size_t table_size_original = tmp_io->read_u32_le();
    size_t block_count = tmp_io->read_u32_le();

    tmp_io.reset(new io::BufferedIO);
    for (auto i : util::range(block_count))
        tmp_io->write_from_io(*reader.read_block());

    tmp_io->seek(0);
    tmp_io.reset(
        new io::BufferedIO(
            util::pack::zlib_inflate(tmp_io->read(table_size_compressed))));

    for (auto &dir_entry : dir_entries)
    {
        auto dn = get_dir_name(*dir_entry, user_fn_map);
        if (dn.size() > 0 && dn[dn.size() - 1] != '/')
            dn += "/";

        for (auto j : util::range(dir_entry->file_count))
        {
            auto fn = util::sjis_to_utf8(tmp_io->read_to_zero()).str();
            auto hash = get_file_name_hash(
                fn, version, dir_entry->initial_hash);
            fn_map[hash] = dn + fn;
        }
    }
    return fn_map;
}

static Table read_table(
    RsaReader &reader, HashLookupMap user_fn_map, TfpkVersion version)
{
    HashLookupMap fn_map;

    //TH135 contains file hashes, TH145 contains garbage
    auto dir_entries = read_dir_entries(reader);
    if (dir_entries.size() > 0)
        fn_map = read_fn_map(reader, dir_entries, user_fn_map, version);

    for (auto &it : user_fn_map)
        fn_map[it.first] = it.second;

    size_t file_count = reader.read_block()->read_u32_le();

    Table table;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        auto b1 = reader.read_block();
        auto b2 = reader.read_block();
        auto b3 = reader.read_block();
        if (version == TfpkVersion::Th135)
        {
            entry->size = b1->read_u32_le();
            entry->offset = b1->read_u32_le();

            auto fn_hash = b2->read_u32_le();
            auto it = fn_map.find(fn_hash);
            entry->name = it == fn_map.end()
                ? get_unknown_name(i, fn_hash)
                : it->second;

            entry->key = b3->read(16);
        }
        else
        {
            entry->size   = b1->read_u32_le() ^ b3->read_u32_le();
            entry->offset = b1->read_u32_le() ^ b3->read_u32_le();

            u32 fn_hash = b2->read_u32_le() ^ b3->read_u32_le();
            u32 unk = b2->read_u32_le() ^ b3->read_u32_le();
            auto it = fn_map.find(fn_hash);
            entry->name = it == fn_map.end()
                ? get_unknown_name(i, fn_hash)
                : it->second;

            b3->seek(0);
            io::BufferedIO key_io;
            for (auto j : util::range(4))
                key_io.write_u32_le(neg32(b3->read_u32_le()));

            key_io.seek(0);
            entry->key = key_io.read_to_eof();
        }
        table.push_back(std::move(entry));
    }

    auto table_end = reader.tell();
    for (auto &entry : table)
        entry->offset += table_end;

    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, TfpkVersion version)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);
    size_t key_size = entry.key.size();
    if (version == TfpkVersion::Th135)
    {
        for (auto i : util::range(entry.size))
            data[i] ^= entry.key[i % key_size];
    }
    else
    {
        auto *key = entry.key.get<const u8>();
        auto *buf = data.get<u8>();
        u8 aux[4];
        for (auto i : util::range(4))
            aux[i] = key[i];
        for (auto i : util::range(entry.size))
        {
            u8 tmp = buf[i];
            buf[i] = tmp ^ key[i % key_size] ^ aux[i%4];
            aux[i%4] = tmp;
        }
    }
    file->io.write(data);
    file->name = entry.name;
    file->guess_extension();
    return file;
}

static void register_palettes(
    const boost::filesystem::path &arc_path,
    HashLookupMap &user_fn_map,
    TfpkVersion version,
    TfbmConverter &converter)
{
    auto dir = boost::filesystem::path(arc_path).parent_path();
    for (boost::filesystem::directory_iterator it(dir);
        it != boost::filesystem::directory_iterator();
        it++)
    {
        if (!boost::filesystem::is_regular_file(it->path()))
            continue;
        if (it->path().string().find(".pak") == std::string::npos)
            continue;

        try
        {
            io::FileIO sub_arc_io(it->path(), io::FileMode::Read);
            if (sub_arc_io.read(magic.size()) != magic)
                continue;
            sub_arc_io.skip(1);
            RsaReader reader(sub_arc_io);
            for (auto &entry : read_table(reader, user_fn_map, version))
            {
                if (entry->name.find("palette") == std::string::npos)
                    continue;
                auto pal_file = read_file(sub_arc_io, *entry, version);
                pal_file->io.seek(0);
                converter.add_palette(entry->name, pal_file->io.read_to_eof());
            }
        }
        catch (...)
        {
            continue;
        }
    }
}

struct TfpkArchive::Priv
{
    TfbmConverter tfbm_converter;
    TfcsConverter tfcs_converter;
    TfwaConverter tfwa_converter;
    fmt::microsoft::DdsConverter dds_converter;
    std::set<std::string> fn_set;
};

void TfpkArchive::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--file-names"})
        ->set_value_name("PATH")
        ->set_description(
            "Specifies path to file containing list of game's file names.\n"
            "Used to get proper file names and to find palettes for sprites.");

    Archive::register_cli_options(arg_parser);
}

void TfpkArchive::parse_cli_options(const ArgParser &arg_parser)
{
    auto path = arg_parser.get_switch("file-names");
    if (path != "")
    {
        io::FileIO io(path, io::FileMode::Read);
        std::string line;
        while ((line = io.read_line().str()) != "")
            p->fn_set.insert(line);
    }

    Archive::parse_cli_options(arg_parser);
}

TfpkArchive::TfpkArchive() : p(new Priv)
{
    add_transformer(&p->tfbm_converter);
    add_transformer(&p->tfcs_converter);
    add_transformer(&p->tfwa_converter);
    add_transformer(&p->dds_converter);
}

TfpkArchive::~TfpkArchive()
{
}

bool TfpkArchive::is_recognized_internal(File &arc_file) const
{
    if (arc_file.io.read(magic.size()) != magic)
        return false;
    auto version = arc_file.io.read_u8();
    return version == 0 || version == 1;
}

void TfpkArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    auto version = arc_file.io.read_u8() == 0
        ? TfpkVersion::Th135
        : TfpkVersion::Th145;

    HashLookupMap user_fn_map;
    for (auto &fn : p->fn_set)
        user_fn_map[get_file_name_hash(fn, version)] = fn;

    register_palettes(arc_file.name, user_fn_map, version, p->tfbm_converter);

    RsaReader reader(arc_file.io);
    Table table = read_table(reader, user_fn_map, version);

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry, version));
}

static auto dummy = fmt::Registry::add<TfpkArchive>("th/tfpk");
