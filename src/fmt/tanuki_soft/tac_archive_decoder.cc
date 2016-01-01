#include "fmt/tanuki_soft/tac_archive_decoder.h"
#include "algo/crypt/blowfish.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

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
    algo::crypt::Blowfish bf(key);
    size_t left = (size / bf.block_size()) * bf.block_size();
    return bf.decrypt(input.substr(0, left)) + input.substr(left);
}

static Version read_version(io::IStream &stream)
{
    if (stream.read(magic_100.size()) == magic_100)
        return Version::Version100;
    stream.seek(0);
    if (stream.read(magic_110.size()) == magic_110)
        return Version::Version110;
    return Version::Unknown;
}

bool TacArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return read_version(input_file.stream) != Version::Unknown;
}

std::unique_ptr<fmt::ArchiveMeta> TacArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto version = read_version(input_file.stream);
    input_file.stream.skip(8);
    size_t entry_count = input_file.stream.read_u32_le();
    size_t dir_count = input_file.stream.read_u32_le();
    size_t table_size = input_file.stream.read_u32_le();
    input_file.stream.skip(4);
    if (version == Version::Version110)
        input_file.stream.skip(8);
    size_t file_data_start = input_file.stream.tell() + table_size;

    auto table_data = input_file.stream.read(table_size);
    table_data = decrypt(table_data, table_size, "TLibArchiveData"_b);
    table_data = algo::pack::zlib_inflate(table_data);
    io::MemoryStream table_stream(table_data);

    std::vector<std::unique_ptr<Directory>> dirs;
    for (auto i : algo::range(dir_count))
    {
        auto dir = std::make_unique<Directory>();
        dir->hash = table_stream.read_u16_le();
        dir->entry_count = table_stream.read_u16_le();
        dir->start_index = table_stream.read_u32_le();
        dirs.push_back(std::move(dir));
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(entry_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->hash = table_stream.read_u64_le();
        entry->compressed = table_stream.read_u32_le() != 0;
        entry->size_original = table_stream.read_u32_le();
        entry->offset = table_stream.read_u32_le() + file_data_start;
        entry->size_compressed = table_stream.read_u32_le();
        entry->path = algo::format("%05d.dat", i);
        meta->entries.push_back(std::move(entry));
    }

    for (auto &dir : dirs)
    {
        for (auto i : algo::range(dir->entry_count))
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

std::unique_ptr<io::File> TacArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_compressed);
    if (entry->compressed)
        data = algo::pack::zlib_inflate(data);

    if (!entry->compressed)
    {
        bstr key = algo::format("%llu_tlib_secure_", entry->hash);
        size_t bytes_to_decrypt = 10240;
        if (data.size() < bytes_to_decrypt)
            bytes_to_decrypt = data.size();

        {
            auto header = decrypt(
                data.substr(0, algo::crypt::Blowfish::block_size()),
                algo::crypt::Blowfish::block_size(),
                key).substr(0, 4);
            if (header == "RIFF"_b || header == "TArc"_b)
                bytes_to_decrypt = data.size();
        }

        data = decrypt(data, bytes_to_decrypt, key);
    }

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> TacArchiveDecoder::get_linked_formats() const
{
    return {"tanuki/tac"};
}

static auto dummy = fmt::register_fmt<TacArchiveDecoder>("tanuki/tac");
