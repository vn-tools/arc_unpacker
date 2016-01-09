#include "dec/alice_soft/ald_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::alice_soft;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static u32 read_24_le(io::IStream &input_stream)
{
    return (input_stream.read<u8>() << 8)
        | (input_stream.read<u8>() << 16)
        | (input_stream.read<u8>() << 24);
}

bool AldArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("ald");
}

std::unique_ptr<dec::ArchiveMeta> AldArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto file_count = read_24_le(input_file.stream) / 3;

    std::vector<size_t> offsets(file_count);
    for (const auto i : algo::range(file_count))
        offsets[i] = read_24_le(input_file.stream);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto &offset : offsets)
    {
        if (!offset)
            break;
        input_file.stream.seek(offset);
        const auto header_size = input_file.stream.read_le<u32>();
        if (input_file.stream.tell() + header_size < input_file.stream.size())
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->size = input_file.stream.read_le<u32>();
            input_file.stream.skip(8);
            const auto name = input_file.stream.read_to_zero(header_size - 16);
            entry->path = algo::sjis_to_utf8(name).str();
            entry->offset = input_file.stream.tell();
            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

std::unique_ptr<io::File> AldArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AldArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/pms", "alice-soft/vsp", "alice-soft/qnt"};
}

static auto _ = dec::register_decoder<AldArchiveDecoder>("alice-soft/ald");
