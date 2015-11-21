#include "fmt/amuse_craft/pac_archive_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static const bstr magic = "PAC\x20"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static int detect_version(io::Stream &stream)
{
    try
    {
        stream.seek(0);
        const auto file_count = stream.read_u16_le();
        stream.seek(0x3FE);
        stream.skip((file_count - 1) * (16 + 8));
        stream.skip(16);
        const auto last_entry_offset = stream.read_u32_le();
        const auto last_entry_size = stream.read_u32_le();
        if (last_entry_offset + last_entry_size == stream.size())
            return 1;
    }
    catch (...) { }

    try
    {
        stream.seek(magic.size() + 4);
        const auto file_count = stream.read_u32_le();
        stream.seek(0x804);
        stream.skip((file_count - 1) * (32 + 8));
        stream.skip(32);
        const auto last_entry_offset = stream.read_u32_le();
        const auto last_entry_size = stream.read_u32_le();
        if (last_entry_offset + last_entry_size == stream.size())
            return 2;
    }
    catch (...) { }

    throw err::RecognitionError();
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta(
    io::File &input_file, const size_t file_count, const size_t name_size)
{
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = input_file.stream.read_to_zero(name_size).str();
        std::replace(entry->name.begin(), entry->name.end(), '_', '/');
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

bool PacArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return detect_version(input_file.stream) > 0;
}

std::unique_ptr<fmt::ArchiveMeta>
    PacArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    const auto version = detect_version(input_file.stream);
    if (version == 1)
    {
        input_file.stream.seek(0);
        const auto file_count = input_file.stream.read_u16_le();
        input_file.stream.seek(0x3FE);
        return ::read_meta(input_file, file_count, 16);
    }
    else if (version == 2)
    {
        input_file.stream.seek(magic.size() + 4);
        const auto file_count = input_file.stream.read_u32_le();
        input_file.stream.seek(0x804);
        return ::read_meta(input_file, file_count, 32);
    }
    else
        throw err::UnsupportedVersionError(version);
}

std::unique_ptr<io::File> PacArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    const auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->name, data);
}

std::vector<std::string> PacArchiveDecoder::get_linked_formats() const
{
    return
    {
        "truevision/tga",
        "amuse-craft/pgd-ge",
        "amuse-craft/pgd-c00",
        "amuse-craft/bgm",
    };
}

static auto dummy = fmt::register_fmt<PacArchiveDecoder>("amuse-craft/pac");
