#include "fmt/alice_soft/ald_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static u32 read_24_le(io::Stream &stream)
{
    return (stream.read_u8() << 8)
        | (stream.read_u8() << 16)
        | (stream.read_u8() << 24);
}

bool AldArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.has_extension("ald");
}

std::unique_ptr<fmt::ArchiveMeta>
    AldArchiveDecoder::read_meta_impl(File &input_file) const
{
    auto file_count = read_24_le(input_file.stream) / 3;

    std::vector<size_t> offsets(file_count);
    for (auto i : util::range(file_count))
        offsets[i] = read_24_le(input_file.stream);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto &offset : offsets)
    {
        if (!offset)
            break;
        input_file.stream.seek(offset);
        auto header_size = input_file.stream.read_u32_le();
        if (input_file.stream.tell() + header_size < input_file.stream.size())
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->size = input_file.stream.read_u32_le();
            input_file.stream.skip(8);
            auto name = input_file.stream.read_to_zero(header_size - 16);
            entry->name = util::sjis_to_utf8(name).str();
            entry->offset = input_file.stream.tell();
            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

std::unique_ptr<File> AldArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AldArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/pms", "alice-soft/vsp", "alice-soft/qnt"};
}

static auto dummy = fmt::register_fmt<AldArchiveDecoder>("alice-soft/ald");
