#include "fmt/qlie/abmp7_archive_decoder.h"
#include "algo/locale.h"

using namespace au;
using namespace au::fmt::qlie;

static const bstr magic = "ABMP7"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Abmp7ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta> Abmp7ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(12);
    input_file.stream.skip(input_file.stream.read_u32_le());

    auto meta = std::make_unique<ArchiveMeta>();

    auto first_entry = std::make_unique<ArchiveEntryImpl>();
    first_entry->path = "base.dat";
    first_entry->size = input_file.stream.read_u32_le();
    first_entry->offset = input_file.stream.tell();
    input_file.stream.skip(first_entry->size);
    meta->entries.push_back(std::move(first_entry));

    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto encoded_name = input_file.stream.read(input_file.stream.read_u8());
        input_file.stream.skip(31 - encoded_name.size());
        entry->path = algo::sjis_to_utf8(encoded_name).str();
        if (entry->path.str().empty())
            entry->path = "unknown";
        entry->path.change_extension("dat");
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.tell();
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Abmp7ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<Abmp7ArchiveDecoder>("qlie/abmp7");
