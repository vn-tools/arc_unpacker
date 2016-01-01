#include "dec/entis/noa_archive_decoder.h"
#include "algo/range.h"
#include "dec/entis/common/sections.h"

using namespace au;
using namespace au::dec::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x04\x00\x02\x00\x00\x00\x00"_b;
static const bstr magic3 = "ERISA-Archive file"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool encrypted;
        bstr extra;
    };
}

static std::unique_ptr<dec::ArchiveMeta> read_meta(
    io::IStream &input_stream, const io::path root = "")
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    common::SectionReader section_reader(input_stream);
    for (auto &section : section_reader.get_sections("DirEntry"))
    {
        input_stream.seek(section.offset);
        auto entry_count = input_stream.read_u32_le();
        for (auto i : algo::range(entry_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->size = input_stream.read_u64_le();
            auto flags = input_stream.read_u32_le();
            entry->encrypted = input_stream.read_u32_le() > 0;
            entry->offset = section.offset + input_stream.read_u64_le();
            input_stream.skip(8);

            auto extra_size = input_stream.read_u32_le();
            if (flags & 0x70)
                entry->extra = input_stream.read(extra_size);

            const auto file_name_size = input_stream.read_u32_le();
            entry->path = input_stream.read_to_zero(file_name_size).str();
            if (!root.str().empty())
                entry->path = root / entry->path;

            if (flags == 0x10)
            {
                const auto sub_meta = read_meta(input_stream, entry->path);
                input_stream.peek(entry->offset, [&]()
                {
                    for (auto &sub_entry : sub_meta->entries)
                        meta->entries.push_back(std::move(sub_entry));
                });
            }
            else if (flags == 0x20 || flags == 0x40)
            {
            }
            else
            {
                meta->entries.push_back(std::move(entry));
            }
        }
    }
    return meta;
}

bool NoaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic1.size()) == magic1
        && input_file.stream.read(magic2.size()) == magic2
        && input_file.stream.read(magic3.size()) == magic3;
}

std::unique_ptr<dec::ArchiveMeta> NoaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x40);
    return ::read_meta(input_file.stream);
}

std::unique_ptr<io::File> NoaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->encrypted)
    {
        logger.warn(
            "%s is encrypted, but encrypted files are not supported\n",
            entry->path.c_str());
    }
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> NoaArchiveDecoder::get_linked_formats() const
{
    return {"entis/mio", "entis/eri"};
}

static auto _ = dec::register_decoder<NoaArchiveDecoder>("entis/noa");
