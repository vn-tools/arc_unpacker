#include "fmt/team_shanghai_alice/pbgz_archive_decoder.h"
#include <map>
#include "err.h"
#include "fmt/team_shanghai_alice/crypt.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static const bstr magic = "PBGZ"_b;
static const bstr crypt_magic = "edz"_b;
static const bstr jpeg_magic = "\xFF\xD8\xFF\xE0"_b;

namespace
{
    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        size_t encryption_version;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

static std::vector<std::map<u8, DecryptorContext>> decryptors
{
    {
        { 0x2A, { 0x99, 0x37,  0x400, 0x1000 } },
        { 0x2D, { 0x35, 0x97,   0x80, 0x2800 } },
        { 0x41, { 0xC1, 0x51,  0x400,  0x400 } },
        { 0x45, { 0xAB, 0xCD,  0x200, 0x1000 } },
        { 0x4A, { 0x03, 0x19,  0x400,  0x400 } },
        { 0x4D, { 0x1B, 0x37,   0x40, 0x2800 } },
        { 0x54, { 0x51, 0xE9,   0x40, 0x3000 } },
        { 0x57, { 0x12, 0x34,  0x400,  0x400 } },
    },
    {
        { 0x2A, { 0x99, 0x37,  0x400, 0x1000 } },
        { 0x2D, { 0x35, 0x97,   0x80, 0x2800 } },
        { 0x41, { 0xC1, 0x51, 0x1400, 0x2000 } },
        { 0x45, { 0xAB, 0xCD,  0x200, 0x1000 } },
        { 0x4A, { 0x03, 0x19, 0x1400, 0x7800 } },
        { 0x4D, { 0x1B, 0x37,   0x40, 0x2000 } },
        { 0x54, { 0x51, 0xE9,   0x40, 0x3000 } },
        { 0x57, { 0x12, 0x34,  0x400, 0x2800 } },
    },
};

static bstr decompress(const bstr &input, size_t size_orig)
{
    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    return util::pack::lzss_decompress_bitwise(input, size_orig, settings);
}

static std::unique_ptr<File> read_file(
    File &arc_file, const fmt::ArchiveEntry &e, u8 encryption_version)
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    arc_file.io.seek(entry->offset);
    io::BufferedIO uncompressed_io(
        decompress(
            arc_file.io.read(entry->size_comp),
            entry->size_orig));

    if (uncompressed_io.read(crypt_magic.size()) != crypt_magic)
        throw err::NotSupportedError("Unknown encryption");

    auto data = decrypt(
        uncompressed_io.read_to_eof(),
        decryptors[encryption_version][uncompressed_io.read_u8()]);

    return std::make_unique<File>(entry->name, data);
}

static size_t detect_encryption_version(
    File &arc_file, const fmt::ArchiveMeta &meta)
{
    for (auto &entry : meta.entries)
    {
        if (entry->name.find(".jpg") == std::string::npos)
            continue;
        for (auto version : util::range(decryptors.size()))
        {
            auto file = read_file(arc_file, *entry, version);
            file->io.seek(0);
            if (file->io.read(jpeg_magic.size()) == jpeg_magic)
                return version;
        }
    }
    throw err::NotSupportedError("No means to detect the encryption version");
}

bool PbgzArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PbgzArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());

    io::BufferedIO header_io(
        decrypt(arc_file.io.read(12), { 0x1B, 0x37, 0x0C, 0x400 }));
    auto file_count = header_io.read_u32_le() - 123456;
    auto table_offset = header_io.read_u32_le() - 345678;
    auto table_size_orig = header_io.read_u32_le() - 567891;

    arc_file.io.seek(table_offset);
    io::BufferedIO table_io(
        decompress(
            decrypt(arc_file.io.read_to_eof(), { 0x3E, 0x9B, 0x80, 0x400 }),
            table_size_orig));

    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMetaImpl>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read_to_zero().str();
        entry->offset = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        table_io.skip(4);
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
        last_entry->size_comp = table_offset - last_entry->offset;

    meta->encryption_version = detect_encryption_version(arc_file, *meta);
    return std::move(meta);
}

std::unique_ptr<File> PbgzArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    return ::read_file(arc_file, e, meta->encryption_version);
}

std::vector<std::string> PbgzArchiveDecoder::get_linked_formats() const
{
    return { "team-shanghai-alice/anm" };
}

static auto dummy
    = fmt::register_fmt<PbgzArchiveDecoder>("team-shanghai-alice/pbgz");
