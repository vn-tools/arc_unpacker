#include "fmt/innocent_grey/iga_archive_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::innocent_grey;

static const bstr magic = "IGA0"_b;

namespace
{
    struct EntrySpec final
    {
        size_t name_offset;
        size_t name_size;
        size_t data_offset;
        size_t data_size;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static u32 read_integer(io::Stream &stream)
{
    u32 ret = 0;
    while (!(ret & 1))
    {
        ret <<= 7;
        ret |= stream.read_u8();
    }
    return ret >> 1;
}

bool IgaArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    IgaArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(12);

    const auto table_size = read_integer(input_file.stream);
    const auto table_start = input_file.stream.tell();
    const auto table_end = table_start + table_size;
    EntrySpec *last_entry_spec = nullptr;
    std::vector<std::unique_ptr<EntrySpec>> entry_specs;
    while (input_file.stream.tell() < table_end)
    {
        auto spec = std::make_unique<EntrySpec>();
        spec->name_offset = read_integer(input_file.stream);
        spec->data_offset = read_integer(input_file.stream);
        spec->data_size = read_integer(input_file.stream);
        if (last_entry_spec)
        {
            last_entry_spec->name_size
                = spec->name_offset - last_entry_spec->name_offset;
        }
        last_entry_spec = spec.get();
        entry_specs.push_back(std::move(spec));
    }
    if (!last_entry_spec)
        return std::make_unique<ArchiveMeta>();

    const auto names_size = read_integer(input_file.stream);
    const auto names_start = input_file.stream.tell();
    last_entry_spec->name_size = names_size - last_entry_spec->name_offset;

    const auto data_offset = input_file.stream.size()
        - last_entry_spec->data_offset
        - last_entry_spec->data_size;

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto &spec : entry_specs)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        input_file.stream.seek(names_start + spec->name_offset);
        for (auto i : util::range(spec->name_size))
            entry->name += read_integer(input_file.stream);
        entry->offset = data_offset + spec->data_offset;
        entry->size = spec->data_size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> IgaArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    for (auto i : util::range(data.size()))
        data[i] ^= (i + 2) & 0xFF;
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<IgaArchiveDecoder>("innocent-grey/iga");
