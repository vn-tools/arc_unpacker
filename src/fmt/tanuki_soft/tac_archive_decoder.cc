#include "fmt/tanuki_soft/tac_archive_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/crypt/blowfish.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::tanuki_soft;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        u64 hash;
        bool compressed;
        u32 size_original;
        u32 offset;
        u32 size_compressed;
    };

    struct Directory final
    {
        u16 hash;
        u16 entry_count;
        u32 start_index;
    };

    enum Version
    {
        Unknown,
        Version100,
        Version110,
    };
}

static const bstr magic_100 = "TArc1.00\x00\x00\x00\x00"_b;
static const bstr magic_110 = "TArc1.10\x00\x00\x00\x00"_b;

static bstr decrypt(const bstr &input, size_t size, const bstr &key)
{
    util::crypt::Blowfish bf(key);
    size_t left = (size / bf.block_size()) * bf.block_size();
    return bf.decrypt(input.substr(0, left)) + input.substr(left);
}

static Version read_version(io::IO &io)
{
    if (io.read(magic_100.size()) == magic_100)
        return Version::Version100;
    io.seek(0);
    if (io.read(magic_110.size()) == magic_110)
        return Version::Version110;
    return Version::Unknown;
}

TacArchiveDecoder::TacArchiveDecoder()
{
    add_decoder(this);
}

bool TacArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return read_version(arc_file.io) != Version::Unknown;
}

std::unique_ptr<fmt::ArchiveMeta>
    TacArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto version = read_version(arc_file.io);
    arc_file.io.skip(8);
    size_t entry_count = arc_file.io.read_u32_le();
    size_t dir_count = arc_file.io.read_u32_le();
    size_t table_size = arc_file.io.read_u32_le();
    arc_file.io.skip(4);
    if (version == Version::Version110)
        arc_file.io.skip(8);
    size_t file_data_start = arc_file.io.tell() + table_size;

    auto table_data = arc_file.io.read(table_size);
    table_data = decrypt(table_data, table_size, "TLibArchiveData"_b);
    table_data = util::pack::zlib_inflate(table_data);
    io::BufferedIO table_io(table_data);

    std::vector<std::unique_ptr<Directory>> dirs;
    for (auto i : util::range(dir_count))
    {
        auto dir = std::make_unique<Directory>();
        dir->hash = table_io.read_u16_le();
        dir->entry_count = table_io.read_u16_le();
        dir->start_index = table_io.read_u32_le();
        dirs.push_back(std::move(dir));
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(entry_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->hash = table_io.read_u64_le();
        entry->compressed = table_io.read_u32_le();
        entry->size_original = table_io.read_u32_le();
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size_compressed = table_io.read_u32_le();
        entry->name = util::format("%05d.dat", i);
        meta->entries.push_back(std::move(entry));
    }

    for (auto &dir : dirs)
    {
        for (auto i : util::range(dir->entry_count))
        {
            if (i + dir->start_index >= meta->entries.size())
                throw err::CorruptDataError("Corrupt file table");
            auto entry = static_cast<ArchiveEntryImpl*>(
                meta->entries[dir->start_index + i].get());
            entry->hash = (entry->hash << 16) | dir->hash;
        }
    }

    return meta;
}

std::unique_ptr<File> TacArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_compressed);
    if (entry->compressed)
        data = util::pack::zlib_inflate(data);

    if (!entry->compressed)
    {
        bstr key = util::format("%llu_tlib_secure_", entry->hash);
        size_t bytes_to_decrypt = 10240;
        if (data.size() < bytes_to_decrypt)
            bytes_to_decrypt = data.size();

        {
            auto header = decrypt(
                data.substr(0, util::crypt::Blowfish::block_size()),
                util::crypt::Blowfish::block_size(),
                key).substr(0, 4);
            if (header == "RIFF"_b || header == "TArc"_b)
                bytes_to_decrypt = data.size();
        }

        data = decrypt(data, bytes_to_decrypt, key);
    }

    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<TacArchiveDecoder>("tanuki/tac");
