#include "fmt/will/arc_archive_decoder.h"
#include <map>
#include "algo/range.h"
#include "err.h"
#include "fmt/will/wipf_image_archive_decoder.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::fmt::will;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool already_unpacked;
    };

    struct Directory final
    {
        std::string extension;
        size_t offset;
        size_t file_count;
    };
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta(
    io::File &input_file, const std::vector<Directory> &dirs, size_t name_size)
{
    auto min_offset = 4 + dirs.size() * 12;
    for (const auto &dir : dirs)
        min_offset += dir.file_count * (name_size + 8);

    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (const auto &dir : dirs)
    {
        input_file.stream.seek(dir.offset);
        for (const auto i : algo::range(dir.file_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            const auto name = input_file.stream.read_to_zero(name_size).str();
            entry->already_unpacked = false;
            entry->path = name + "." + dir.extension;
            entry->size = input_file.stream.read_u32_le();
            entry->offset = input_file.stream.read_u32_le();

            if (entry->path.str().empty())
                throw err::CorruptDataError("Empty file name");
            if (entry->offset < min_offset)
                throw err::BadDataOffsetError();
            if (entry->offset + entry->size > input_file.stream.size())
                throw err::BadDataOffsetError();

            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    return read_meta(dummy_logger, input_file)->entries.size() > 0;
}

std::unique_ptr<fmt::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto dir_count = input_file.stream.read_u32_le();
    if (dir_count > 100)
        throw err::BadDataSizeError();
    std::vector<Directory> dirs(dir_count);
    for (const auto i : algo::range(dirs.size()))
    {
        dirs[i].extension = input_file.stream.read_to_zero(4).str();
        dirs[i].file_count = input_file.stream.read_u32_le();
        dirs[i].offset = input_file.stream.read_u32_le();
    }

    for (const auto name_size : {9, 13})
    {
        try
        {
            return ::read_meta(input_file, dirs, name_size);
        }
        catch (...)
        {
            continue;
        }
    }

    throw err::CorruptDataError("Failed to read file table");
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->already_unpacked)
        return nullptr;

    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    if (entry->path.has_extension("wsc") || entry->path.has_extension("scr"))
    {
        for (auto &c : data)
            c = (c >> 2) | (c << 6);
    }

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"will/wipf"};
}

static auto dummy = fmt::register_fmt<ArcArchiveDecoder>("will/arc");
