#include "fmt/fc01/mrg_archive_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "fmt/fc01/common/custom_lzss.h"
#include "fmt/fc01/common/mrg_decryptor.h"
#include "fmt/fc01/common/util.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "MRG\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        u32 offset;
        u32 size_orig;
        u32 size_comp;
        u8 filter;
    };
}

static u8 guess_key(const bstr &table_data, size_t file_size)
{
    u8 tmp = common::rol8(table_data.get<u8>()[table_data.size() - 1], 1);
    u8 key = tmp ^ (file_size >> 24);
    u32 pos = 1;
    u32 last_offset = tmp ^ key;
    for (auto i = table_data.size() - 2; i >= table_data.size() - 4; --i)
    {
        key -= ++pos;
        tmp = common::rol8(table_data.get<u8>()[i], 1);
        last_offset = (last_offset << 8) | (tmp ^ key);
    }
    if (last_offset != file_size)
        throw err::NotSupportedError("Failed to guess the key");
    while (pos++ < table_data.size())
        key -= pos;
    return key;
}

bool MrgArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta> MrgArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto table_size = input_file.stream.read_u32_le() - 12 - magic.size();
    const auto file_count = input_file.stream.read_u32_le();

    auto table_data = input_file.stream.read(table_size);
    auto key = guess_key(table_data, input_file.stream.size());
    for (auto i : algo::range(table_data.size()))
    {
        table_data[i] = common::rol8(table_data[i], 1) ^ key;
        key += table_data.size() - i;
    }

    io::MemoryStream table_stream(table_data);
    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = table_stream.read_to_zero(0x0E).str();
        entry->size_orig = table_stream.read_u32_le();
        entry->filter = table_stream.read_u8();
        table_stream.skip(9);
        entry->offset = table_stream.read_u32_le();
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
    {
        table_stream.skip(0x1C);
        last_entry->size_comp = table_stream.read_u32_le() - last_entry->offset;
    }

    return meta;
}

std::unique_ptr<io::File> MrgArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);
    if (entry->filter)
    {
        if (entry->filter >= 2)
        {
            common::MrgDecryptor decryptor(data);
            data = decryptor.decrypt_without_key();
        }
        if (entry->filter < 3)
            data = common::custom_lzss_decompress(data, entry->size_orig);
    }
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> MrgArchiveDecoder::get_linked_formats() const
{
    return {"fc01/acd", "fc01/mca", "fc01/mcg"};
}

static auto dummy = fmt::register_fmt<MrgArchiveDecoder>("fc01/mrg");
