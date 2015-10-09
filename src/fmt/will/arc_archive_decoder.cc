#include "fmt/will/arc_archive_decoder.h"
#include <map>
#include "err.h"
#include "fmt/will/wipf_image_archive_decoder.h"
#include "util/file_from_grid.h"
#include "util/range.h"

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
    File &arc_file, const std::vector<Directory> &dirs, size_t name_size)
{
    auto min_offset = 4 + dirs.size() * 12;
    for (auto &dir : dirs)
        min_offset += dir.file_count * (name_size + 8);

    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto &dir : dirs)
    {
        arc_file.io.seek(dir.offset);
        for (auto i : util::range(dir.file_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            auto name = arc_file.io.read_to_zero(name_size).str();
            entry->already_unpacked = false;
            entry->name = name + "." + dir.extension;
            entry->size = arc_file.io.read_u32_le();
            entry->offset = arc_file.io.read_u32_le();

            if (!entry->name.size())
                throw err::CorruptDataError("Empty file name");
            if (entry->offset < min_offset)
                throw err::BadDataOffsetError();
            if (entry->offset + entry->size > arc_file.io.size())
                throw err::BadDataOffsetError();

            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

bool ArcArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return read_meta(arc_file)->entries.size() > 0;
}

void ArcArchiveDecoder::preprocess(
    File &arc_file, fmt::ArchiveMeta &meta, const FileSaver &saver) const
{
    // apply image masks to original sprites
    std::map<std::string, ArchiveEntryImpl*> mask_entries, sprite_entries;
    for (auto &entry : meta.entries)
    {
        auto fn = entry->name.substr(0, entry->name.find_first_of('.'));
        if (entry->name.find("MSK") != std::string::npos)
            mask_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
        else if (entry->name.find("WIP") != std::string::npos)
            sprite_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
    }

    WipfImageArchiveDecoder wipf_archive_decoder;
    for (auto it : sprite_entries)
    {
        try
        {
            auto sprite_entry = it.second;
            auto mask_entry = mask_entries.at(it.first);
            auto sprites = wipf_archive_decoder.unpack_to_images(
                *read_file(arc_file, meta, *sprite_entry));
            auto masks = wipf_archive_decoder.unpack_to_images(
                *read_file(arc_file, meta, *mask_entry));
            for (auto i : util::range(sprites.size()))
                sprites[i]->apply_alpha_from_mask(*masks.at(i));
            sprite_entry->already_unpacked = true;
            mask_entry->already_unpacked = true;
            for (auto &sprite : sprites)
            {
                saver.save(util::file_from_grid(
                    *sprite, sprite_entry->name));
            }
        }
        catch (...)
        {
            continue;
        }
    }
}

std::unique_ptr<fmt::ArchiveMeta>
    ArcArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto dir_count = arc_file.io.read_u32_le();
    if (dir_count > 100)
        throw err::BadDataSizeError();
    std::vector<Directory> dirs(dir_count);
    for (auto i : util::range(dirs.size()))
    {
        dirs[i].extension = arc_file.io.read_to_zero(4).str();
        dirs[i].file_count = arc_file.io.read_u32_le();
        dirs[i].offset = arc_file.io.read_u32_le();
    }

    for (auto name_size : {9, 13})
    {
        try
        {
            return ::read_meta(arc_file, dirs, name_size);
        }
        catch (...)
        {
            continue;
        }
    }

    throw err::CorruptDataError("Failed to read file table");
}

std::unique_ptr<File> ArcArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);

    auto output_file = std::make_unique<File>();
    output_file->name = entry->name;

    if (output_file->has_extension("wsc") || output_file->has_extension("scr"))
        for (auto &c : data)
            c = (c >> 2) | (c << 6);

    output_file->io.write(data);
    return output_file;
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return { "will/wipf" };
}

static auto dummy = fmt::register_fmt<ArcArchiveDecoder>("will/arc");
