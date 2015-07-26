// TAC archive
//
// Company:   Tanuki Soft
// Engine:    -
// Extension: .tac
//
// Known games:
// - Mebae

#include <cinttypes>
#include <openssl/blowfish.h>
#include "io/buffered_io.h"
#include "formats/tanuki_soft/tac_archive.h"
#include "util/itos.h"
#include "util/zlib.h"
#include "util/encoding.h"

using namespace au;
using namespace au::fmt::tanuki_soft;

namespace
{
    struct TableDirectory;

    struct TableEntry
    {
        u64 hash;
        bool compressed;
        u32 size_original;
        u32 offset;
        u32 size_compressed;
        struct TableDirectory *directory;
        size_t index;
    };

    struct TableDirectory
    {
        u16 hash;
        u16 entry_count;
        u32 start_index;
        std::vector<std::unique_ptr<TableEntry>> entries;
    };
}

static const std::string magic("TArc1.00\x00\x00\x00\x00", 12);

static std::string decrypt(
    const std::string &input, size_t size, const std::string &key)
{
    std::string output;

    std::unique_ptr<BF_KEY> bf_key(new BF_KEY);
    BF_set_key(
        bf_key.get(), key.size(), reinterpret_cast<const u8*>(key.data()));

    size_t left = (size / BF_BLOCK) * BF_BLOCK;
    size_t done = 0;

    BF_LONG transit;
    while (left)
    {
        memcpy(&transit, input.data() + done, BF_BLOCK);
        BF_decrypt(&transit, bf_key.get());
        output += std::string(reinterpret_cast<char*>(&transit), BF_BLOCK);
        left -= BF_BLOCK;
        done += BF_BLOCK;
    }
    if (done < input.size())
        output += std::string(input.data() + done, input.size() - done);

    return output;
}

static std::vector<std::unique_ptr<TableDirectory>> read_table(io::IO &arc_io)
{
    size_t entry_count = arc_io.read_u32_le();
    size_t directory_count = arc_io.read_u32_le();
    size_t table_size = arc_io.read_u32_le();
    arc_io.skip(4);
    size_t file_data_start = arc_io.tell() + table_size;

    io::BufferedIO table_io(
        util::zlib_inflate(
            decrypt(arc_io.read(table_size), table_size, "TLibArchiveData")));

    std::vector<std::unique_ptr<TableDirectory>> directories;
    for (size_t i = 0; i < directory_count; i++)
    {
        std::unique_ptr<TableDirectory> dir(new TableDirectory);
        dir->hash = table_io.read_u16_le();
        dir->entry_count = table_io.read_u16_le();
        dir->start_index = table_io.read_u32_le();
        directories.push_back(std::move(dir));
    }

    std::vector<std::unique_ptr<TableEntry>> entries;
    for (size_t i = 0; i < entry_count; i++)
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
        for (size_t i = 0; i < directory->entry_count; i++)
        {
            if (i + directory->start_index >= entries.size())
                throw std::runtime_error("Corrupt file table");
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
    std::string data = arc_io.read(entry.size_compressed);
    if (entry.compressed)
        data = util::zlib_inflate(data);

    if (!entry.compressed)
    {
        char key[40];
        sprintf(key, "%" PRIu64 "_tlib_secure_", entry.hash);

        size_t bytes_to_decrypt = 10240;
        if (data.size() < bytes_to_decrypt)
            bytes_to_decrypt = data.size();

        {
            auto header = decrypt(data.substr(0, BF_BLOCK), BF_BLOCK, key);
            auto header4 = header.substr(0, 4);
            if (header4 == "RIFF" || header4 == "TArc")
                bytes_to_decrypt = data.size();
        }

        data = decrypt(data, bytes_to_decrypt, key);
    }

    file->io.write(data);
    file->name = util::itos(entry.index);
    file->guess_extension();
    return file;
}

TacArchive::TacArchive()
{
    add_transformer(this);
}

bool TacArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("tac");
}

void TacArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) == magic)
        arc_file.io.skip(4 * 2);
    else
        arc_file.io.seek(0);

    auto directories = read_table(arc_file.io);
    for (auto &directory : directories)
        for (auto &entry : directory->entries)
            file_saver.save(read_file(arc_file.io, *entry));
}
