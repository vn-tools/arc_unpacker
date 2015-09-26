#include "fmt/tanuki_soft/tac_archive.h"
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
    struct TableDirectory;

    struct TableEntry final
    {
        u64 hash;
        bool compressed;
        u32 size_original;
        u32 offset;
        u32 size_compressed;
        TableDirectory *directory;
        size_t index;
    };

    struct TableDirectory final
    {
        u16 hash;
        u16 entry_count;
        u32 start_index;
        std::vector<std::unique_ptr<TableEntry>> entries;
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

static std::vector<std::unique_ptr<TableDirectory>> read_table(
    io::IO &arc_io, Version version)
{
    size_t entry_count = arc_io.read_u32_le();
    size_t directory_count = arc_io.read_u32_le();
    size_t table_size = arc_io.read_u32_le();
    arc_io.skip(4);
    if (version == Version::Version110)
        arc_io.skip(8);
    size_t file_data_start = arc_io.tell() + table_size;

    io::BufferedIO table_io(
        util::pack::zlib_inflate(
            decrypt(arc_io.read(table_size), table_size, "TLibArchiveData"_b)));

    std::vector<std::unique_ptr<TableDirectory>> directories;
    for (auto i : util::range(directory_count))
    {
        std::unique_ptr<TableDirectory> dir(new TableDirectory);
        dir->hash = table_io.read_u16_le();
        dir->entry_count = table_io.read_u16_le();
        dir->start_index = table_io.read_u32_le();
        directories.push_back(std::move(dir));
    }

    std::vector<std::unique_ptr<TableEntry>> entries;
    for (auto i : util::range(entry_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->hash = table_io.read_u64_le();
        entry->compressed = table_io.read_u32_le();
        entry->size_original = table_io.read_u32_le();
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size_compressed = table_io.read_u32_le();
        entry->index = i;
        entries.push_back(std::move(entry));
    }

    for (auto &directory : directories)
    {
        for (auto i : util::range(directory->entry_count))
        {
            if (i + directory->start_index >= entries.size())
                throw err::CorruptDataError("Corrupt file table");
            auto &entry = entries[directory->start_index + i];
            entry->hash = (entry->hash << 16) | directory->hash;
            entry->directory = directory.get();
            directory->entries.push_back(std::move(entry));
        }
    }

    return directories;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    bstr data = arc_io.read(entry.size_compressed);
    if (entry.compressed)
        data = util::pack::zlib_inflate(data);

    if (!entry.compressed)
    {
        bstr key = util::format("%llu_tlib_secure_", entry.hash);
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

    file->io.write(data);
    file->name = util::format("%05d.dat", entry.index);
    file->guess_extension();
    return file;
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

TacArchive::TacArchive()
{
    add_decoder(this);
}

bool TacArchive::is_recognized_internal(File &arc_file) const
{
    return read_version(arc_file.io) != Version::Unknown;
}

void TacArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto version = read_version(arc_file.io);
    arc_file.io.skip(8);

    auto directories = read_table(arc_file.io, version);
    for (auto &directory : directories)
        for (auto &entry : directory->entries)
            file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<TacArchive>("tanuki/tac");
