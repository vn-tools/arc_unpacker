#include "fmt/touhou/tfpk_archive_decoder.h"
#include <boost/filesystem.hpp>
#include <map>
#include <set>
#include "err.h"
#include "fmt/microsoft/dds_image_decoder.h"
#include "fmt/touhou/tfbm_image_decoder.h"
#include "fmt/touhou/tfcs_file_decoder.h"
#include "fmt/touhou/tfwa_audio_decoder.h"
#include "io/buffered_io.h"
#include "util/crypt/rsa.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static const bstr magic = "TFPK"_b;

namespace
{
    enum class TfpkVersion : u8
    {
        Th135,
        Th145,
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        TfpkVersion version;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t size;
        size_t offset;
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
        RsaReader(io::IO &io);
        ~RsaReader();
        std::unique_ptr<io::BufferedIO> read_block();
        size_t tell() const;

    private:
        std::unique_ptr<io::BufferedIO> decrypt(std::basic_string<u8> input);

        io::IO &io;
        std::unique_ptr<util::crypt::Rsa> rsa;
    };
}

static const std::vector<util::crypt::RsaKey> rsa_keys({
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

RsaReader::RsaReader(io::IO &io) : io(io)
{
    // test chunk = one block with dir count
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

    // no encryption - TH14.5 English patch.
    // First 4 bytes are integer meaning dir count, so the rest should be zero.
    if (test_chunk.substr(4, 12) == "\0\0\0\0\0\0\0\0\0\0\0\0"_b)
        return;

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
    bstr block = rsa
        ? rsa->decrypt(io.read(0x40)).substr(0, 0x20)
        : io.read(0x40).substr(0, 0x20);
    return std::make_unique<io::BufferedIO>(block);
}

static u32 neg32(u32 x)
{
    return static_cast<u32>(-static_cast<s32>(x));
}

static std::string lower_ascii_only(std::string name_utf8)
{
    // while SJIS can use ASCII for encoding multibyte characters,
    // UTF-8 uses the codes 0–127 only for the ASCII characters.
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

static std::string get_dir_name(
    const DirEntry &dir_entry, const HashLookupMap user_fn_map)
{
    auto it = user_fn_map.find(dir_entry.initial_hash);
    if (it != user_fn_map.end())
        return it->second;
    return get_unknown_name(-1, dir_entry.initial_hash, "");
}

static std::vector<DirEntry> read_dir_entries(RsaReader &reader)
{
    std::vector<DirEntry> dirs;
    auto dir_count = reader.read_block()->read_u32_le();
    for (auto i : util::range(dir_count))
    {
        auto tmp_io = reader.read_block();
        DirEntry entry;
        entry.initial_hash = tmp_io->read_u32_le();
        entry.file_count = tmp_io->read_u32_le();
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
        auto dn = get_dir_name(dir_entry, user_fn_map);
        if (dn.size() > 0 && dn[dn.size() - 1] != '/')
            dn += "/";

        for (auto j : util::range(dir_entry.file_count))
        {
            // TH145 English patch contains invalid directory names
            try
            {
                auto fn = util::sjis_to_utf8(tmp_io->read_to_zero()).str();
                auto hash = get_file_name_hash(
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

struct TfpkArchiveDecoder::Priv final
{
    TfbmImageDecoder tfbm_image_decoder;
    TfcsFileDecoder tfcs_file_decoder;
    TfwaAudioDecoder tfwa_audio_decoder;
    fmt::microsoft::DdsImageDecoder dds_image_decoder;
    std::set<std::string> fn_set;
};

void TfpkArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--file-names"})
        ->set_value_name("PATH")
        ->set_description(
            "Specifies path to file containing list of game's file names.\n"
            "Used to get proper file names and to find palettes for sprites.");

    ArchiveDecoder::register_cli_options(arg_parser);
}

void TfpkArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    auto path = arg_parser.get_switch("file-names");
    if (path != "")
    {
        io::FileIO io(path, io::FileMode::Read);
        std::string line;
        while ((line = io.read_line().str()) != "")
            p->fn_set.insert(line);
    }

    ArchiveDecoder::parse_cli_options(arg_parser);
}

TfpkArchiveDecoder::TfpkArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->tfbm_image_decoder);
    add_decoder(&p->tfcs_file_decoder);
    add_decoder(&p->tfwa_audio_decoder);
    add_decoder(&p->dds_image_decoder);
}

TfpkArchiveDecoder::~TfpkArchiveDecoder()
{
}

bool TfpkArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    if (arc_file.io.read(magic.size()) != magic)
        return false;
    auto version = arc_file.io.read_u8();
    return version == 0 || version == 1;
}

std::unique_ptr<fmt::ArchiveMeta>
    TfpkArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(magic.size());

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->version = arc_file.io.read_u8() == 0
        ? TfpkVersion::Th135
        : TfpkVersion::Th145;

    HashLookupMap user_fn_map;
    for (auto &fn : p->fn_set)
        user_fn_map[get_file_name_hash(fn, meta->version)] = fn;

    RsaReader reader(arc_file.io);
    HashLookupMap fn_map;

    // TH135 contains file hashes, TH145 contains garbage
    auto dir_entries = read_dir_entries(reader);
    if (dir_entries.size() > 0)
        fn_map = read_fn_map(reader, dir_entries, user_fn_map, meta->version);

    for (auto &it : user_fn_map)
        fn_map[it.first] = it.second;

    size_t file_count = reader.read_block()->read_u32_le();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto b1 = reader.read_block();
        auto b2 = reader.read_block();
        auto b3 = reader.read_block();
        if (meta->version == TfpkVersion::Th135)
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
        meta->entries.push_back(std::move(entry));
    }

    auto table_end = reader.tell();
    for (auto &entry : meta->entries)
        static_cast<ArchiveEntryImpl*>(entry.get())->offset += table_end;

    return meta;
}

std::unique_ptr<File> TfpkArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    size_t key_size = entry->key.size();
    if (meta->version == TfpkVersion::Th135)
    {
        for (auto i : util::range(entry->size))
            data[i] ^= entry->key[i % key_size];
    }
    else
    {
        auto *key = entry->key.get<const u8>();
        auto *buf = data.get<u8>();
        u8 aux[4];
        for (auto i : util::range(4))
            aux[i] = key[i];
        for (auto i : util::range(entry->size))
        {
            u8 tmp = buf[i];
            buf[i] = tmp ^ key[i % key_size] ^ aux[i%4];
            aux[i%4] = tmp;
        }
    }
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

void TfpkArchiveDecoder::preprocess(
    File &arc_file, ArchiveMeta &m, FileSaver &saver) const
{
    p->tfbm_image_decoder.clear_palettes();
    auto dir = boost::filesystem::path(arc_file.name).parent_path();
    for (boost::filesystem::directory_iterator it(dir);
        it != boost::filesystem::directory_iterator();
        it++)
    {
        if (!boost::filesystem::is_regular_file(it->path()))
            continue;
        if (it->path().string().find(".pak") == std::string::npos)
            continue;

        File other_arc_file(it->path(), io::FileMode::Read);
        if (!is_recognized(other_arc_file))
            continue;

        auto meta = read_meta(other_arc_file);
        for (auto &entry : meta->entries)
        {
            if (entry->name.find("palette") == std::string::npos)
                continue;
            auto pal_file = read_file(other_arc_file, *meta, *entry);
            pal_file->io.seek(0);
            p->tfbm_image_decoder.add_palette(
                entry->name, pal_file->io.read_to_eof());
        }
    }
}

static auto dummy = fmt::Registry::add<TfpkArchiveDecoder>("th/tfpk");
