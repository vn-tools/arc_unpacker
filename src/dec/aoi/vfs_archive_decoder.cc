#include "dec/aoi/vfs_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr magic = "VF"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool VfsArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.seek(0).read(magic.size()) != magic)
        return false;
    const auto version = input_file.stream.read_le<u16>();
    return version == 0x100 || version == 0x101 || version == 0x200;
}

std::unique_ptr<dec::ArchiveMeta> VfsArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(2);
    const auto version = input_file.stream.read_le<u16>();
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.read_le<u16>();
    const auto entry_size = input_file.stream.read_le<u16>();
    const auto table_size = input_file.stream.read_le<u32>();
    const auto file_size = input_file.stream.read_le<u32>();

    if (version == 0x100 || version == 0x101)
    {
        for (const auto i : algo::range(file_count))
        {
            const auto entry_offset = input_file.stream.tell();
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->path = input_file.stream.read_to_zero(0x13).str();
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();
            meta->entries.push_back(std::move(entry));
            input_file.stream.seek(entry_offset + entry_size);
        }
    }

    else if (version == 0x200)
    {
        const auto names_offset
            = input_file.stream.tell() + entry_size * file_count + 8;
        for (const auto i  : algo::range(file_count))
        {
            const auto entry_offset = input_file.stream.tell();
            auto entry = std::make_unique<ArchiveEntryImpl>();

            const auto name_offset
                = names_offset + input_file.stream.read_le<u32>() * 2;
            input_file.stream.skip(6);
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();

            input_file.stream.seek(name_offset);
            bstr name_utf16;
            while (true)
            {
                const auto chunk = input_file.stream.read(2);
                if (chunk == "\x00\x00"_b)
                    break;
                name_utf16 += chunk;
            }
            entry->path = algo::utf16_to_utf8(name_utf16).str();

            meta->entries.push_back(std::move(entry));
            input_file.stream.seek(entry_offset + entry_size);
        }
    }
    else
    {
        throw err::UnsupportedVersionError(version);
    }

    return meta;
}

std::unique_ptr<io::File> VfsArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    return std::make_unique<io::File>(
        entry->path,
        input_file.stream.seek(entry->offset).read(entry->size));
}

std::vector<std::string> VfsArchiveDecoder::get_linked_formats() const
{
    return {"aoi/iph", "aoi/aog", "aoi/agf", "microsoft/dds"};
}

static auto _ = dec::register_decoder<VfsArchiveDecoder>("aoi/vfs");
