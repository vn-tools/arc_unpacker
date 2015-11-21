#include "fmt/alice_soft/afa_archive_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic1 = "AFAH"_b;
static const bstr magic2 = "AlicArch"_b;
static const bstr magic3 = "INFO"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool AfaArchiveDecoder::is_recognized_impl(File &input_file) const
{
    if (input_file.stream.read(magic1.size()) != magic1)
        return false;
    input_file.stream.skip(4);
    return input_file.stream.read(magic2.size()) == magic2;
}

std::unique_ptr<fmt::ArchiveMeta>
    AfaArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic1.size() + 4 + magic2.size() + 4 * 2);
    auto file_data_start = input_file.stream.read_u32_le();
    if (input_file.stream.read(magic3.size()) != magic3)
        throw err::CorruptDataError("Corrupt file table");

    auto table_size_compressed = input_file.stream.read_u32_le();
    auto table_size_original = input_file.stream.read_u32_le();
    auto file_count = input_file.stream.read_u32_le();

    io::MemoryStream table_stream(
        util::pack::zlib_inflate(
            input_file.stream.read(table_size_compressed)));

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        table_stream.skip(4);
        auto name_size = table_stream.read_u32_le();
        entry->name = util::sjis_to_utf8(
            table_stream.read_to_zero(name_size)).str();

        table_stream.skip(4 * 3); // for some games, apparently this is 4 * 2
        entry->offset = table_stream.read_u32_le() + file_data_start;
        entry->size = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> AfaArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AfaArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/aff", "alice-soft/ajp", "alice-soft/qnt"};
}

static auto dummy = fmt::register_fmt<AfaArchiveDecoder>("alice-soft/afa");
