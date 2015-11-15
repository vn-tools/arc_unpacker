#include "fmt/team_shanghai_alice/tha1_archive_decoder.h"
#include "err.h"
#include "fmt/team_shanghai_alice/crypt.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static const bstr magic = "THA1"_b;

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
        u8 decryptor_id;
    };
}

static std::vector<std::vector<DecryptorContext>> decryptors
{
    // TH9.5, TH10, TH11
    {
        {0x1B, 0x37,  0x40, 0x2800},
        {0x51, 0xE9,  0x40, 0x3000},
        {0xC1, 0x51,  0x80, 0x3200},
        {0x03, 0x19, 0x400, 0x7800},
        {0xAB, 0xCD, 0x200, 0x2800},
        {0x12, 0x34,  0x80, 0x3200},
        {0x35, 0x97,  0x80, 0x2800},
        {0x99, 0x37, 0x400, 0x2000},
    },

    // TH12, TH12.5, TH12.8
    {
        {0x1B, 0x73,  0x40, 0x3800},
        {0x51, 0x9E,  0x40, 0x4000},
        {0xC1, 0x15, 0x400, 0x2C00},
        {0x03, 0x91,  0x80, 0x6400},
        {0xAB, 0xDC,  0x80, 0x6E00},
        {0x12, 0x43, 0x200, 0x3C00},
        {0x35, 0x79, 0x400, 0x3C00},
        {0x99, 0x7D,  0x80, 0x2800},
    },

    // TH13
    {
        {0x1B, 0x73, 0x0100, 0x3800},
        {0x12, 0x43, 0x0200, 0x3E00},
        {0x35, 0x79, 0x0400, 0x3C00},
        {0x03, 0x91, 0x0080, 0x6400},
        {0xAB, 0xDC, 0x0080, 0x6E00},
        {0x51, 0x9E, 0x0100, 0x4000},
        {0xC1, 0x15, 0x0400, 0x2C00},
        {0x99, 0x7D, 0x0080, 0x4400},
    },

    // TH14, TH15 trial, TH15
    {
        {0x1B, 0x73, 0x0100, 0x3800},
        {0x12, 0x43, 0x0200, 0x3E00},
        {0x35, 0x79, 0x0400, 0x3C00},
        {0x03, 0x91, 0x0080, 0x6400},
        {0xAB, 0xDC, 0x0080, 0x7000},
        {0x51, 0x9E, 0x0100, 0x4000},
        {0xC1, 0x15, 0x0400, 0x2C00},
        {0x99, 0x7D, 0x0080, 0x4400}
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

static int detect_encryption_version(File &arc_file)
{
    if (arc_file.name.find("th095.") != std::string::npos) return 0;
    if (arc_file.name.find("th10.") != std::string::npos) return 0;
    if (arc_file.name.find("th11.") != std::string::npos) return 0;
    if (arc_file.name.find("th12.") != std::string::npos) return 1;
    if (arc_file.name.find("th125.") != std::string::npos) return 1;
    if (arc_file.name.find("th128.") != std::string::npos) return 1;
    if (arc_file.name.find("th13.") != std::string::npos) return 2;
    if (arc_file.name.find("th14.") != std::string::npos) return 3;
    if (arc_file.name.find("th143.") != std::string::npos) return 3;
    if (arc_file.name.find("th15tr.") != std::string::npos) return 3;
    if (arc_file.name.find("th15.") != std::string::npos) return 3;
    return -1;
}

bool Tha1ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    auto encryption_version = detect_encryption_version(arc_file);
    if (encryption_version < 0)
        return false;
    arc_file.io.seek(0);
    auto header_data = arc_file.io.read(16);
    header_data = decrypt(header_data, {0x1B, 0x37, 0x10, 0x400});
    io::BufferedIO header_io(header_data);
    return header_io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Tha1ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto header_data = arc_file.io.read(16);
    header_data = decrypt(header_data, {0x1B, 0x37, 0x10, 0x400});
    io::BufferedIO header_io(header_data);
    if (header_io.read(magic.size()) != magic)
        throw err::RecognitionError();
    auto table_size_orig = header_io.read_u32_le() - 123456789;
    auto table_size_comp = header_io.read_u32_le() - 987654321;
    auto file_count = header_io.read_u32_le() - 135792468;
    auto table_offset = arc_file.io.size() - table_size_comp;

    arc_file.io.seek(table_offset);
    auto table_data = arc_file.io.read(table_size_comp);
    table_data = decrypt(table_data, {0x3E, 0x9B, 0x80, table_size_comp});
    table_data = decompress(table_data, table_size_orig);
    io::BufferedIO table_io(table_data);

    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMetaImpl>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        entry->name = table_io.read_to_zero().str();
        table_io.skip(3 - entry->name.size() % 4);

        entry->decryptor_id = 0;
        for (auto j : util::range(entry->name.size()))
            entry->decryptor_id += entry->name[j];
        entry->decryptor_id %= 8;

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

    meta->encryption_version = detect_encryption_version(arc_file);
    return std::move(meta);
}

std::unique_ptr<File> Tha1ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);
    data = decrypt(
        data, decryptors[meta->encryption_version][entry->decryptor_id]);
    if (entry->size_comp != entry->size_orig)
        data = decompress(data, entry->size_orig);

    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> Tha1ArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto dummy
    = fmt::register_fmt<Tha1ArchiveDecoder>("team-shanghai-alice/tha1");
