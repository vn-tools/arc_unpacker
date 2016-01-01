#include "fmt/entis/noa_archive_decoder.h"
#include "algo/range.h"
#include "fmt/entis/common/sections.h"

using namespace au;
using namespace au::fmt::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x04\x00\x02\x00\x00\x00\x00"_b;
static const bstr magic3 = "ERISA-Archive file"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool encrypted;
        bstr extra;
    };
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta(
    io::IStream &stream, const io::path root = "")
{
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    common::SectionReader section_reader(stream);
    for (auto &section : section_reader.get_sections("DirEntry"))
    {
        stream.seek(section.offset);
        auto entry_count = stream.read_u32_le();
        for (auto i : algo::range(entry_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->size = stream.read_u64_le();
            auto flags = stream.read_u32_le();
            entry->encrypted = stream.read_u32_le() > 0;
            entry->offset = section.offset + stream.read_u64_le();
            stream.skip(8);

            auto extra_size = stream.read_u32_le();
            if (flags & 0x70)
                entry->extra = stream.read(extra_size);

            entry->path = stream.read_to_zero(stream.read_u32_le()).str();
            if (!root.str().empty())
                entry->path = root / entry->path;

            if (flags == 0x10)
            {
                const auto sub_meta = read_meta(stream, entry->path);
                stream.peek(entry->offset, [&]()
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

std::unique_ptr<fmt::ArchiveMeta> NoaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x40);
    return ::read_meta(input_file.stream);
}

std::unique_ptr<io::File> NoaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
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

static auto dummy = fmt::register_fmt<NoaArchiveDecoder>("entis/noa");
