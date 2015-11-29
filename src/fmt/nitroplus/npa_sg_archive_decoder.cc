#include "fmt/nitroplus/npa_sg_archive_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

static const bstr key = "\xBD\xAA\xBC\xB4\xAB\xB6\xBC\xB4"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(bstr &data)
{
    for (auto i : util::range(data.size()))
        data[i] ^= key[i % key.size()];
}

bool NpaSgArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("npa"))
        return false;
    size_t table_size = input_file.stream.read_u32_le();
    return table_size < input_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    NpaSgArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    size_t table_size = input_file.stream.read_u32_le();
    auto table_data = input_file.stream.read(table_size);
    decrypt(table_data);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    size_t file_count = table_stream.read_u32_le();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto name_size = table_stream.read_u32_le();
        entry->name = util::convert_encoding(
            table_stream.read(name_size), "utf-16le", "utf-8").str();
        entry->size = table_stream.read_u32_le();
        entry->offset = table_stream.read_u32_le();
        table_stream.skip(4);
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> NpaSgArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    decrypt(data);
    return std::make_unique<io::File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<NpaSgArchiveDecoder>("nitroplus/npa-sg");
