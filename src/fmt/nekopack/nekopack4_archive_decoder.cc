#include "fmt/nekopack/nekopack4_archive_decoder.h"
#include "log.h"
#include <map>
#include "err.h"
#include "fmt/microsoft/bmp_image_decoder.h"
#include "io/buffered_io.h"
#include "util/file_from_grid.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nekopack;

static const bstr magic = "NEKOPACK4A"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        u32 offset;
        u32 size_comp;
        bool already_unpacked;
    };
}

bool Nekopack4ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Nekopack4ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto table_size = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto name_size = arc_file.io.read_u32_le();
        if (!name_size)
            break;

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->already_unpacked = false;
        entry->name = arc_file.io.read_to_zero(name_size).str();
        u32 key = 0;
        for (auto &c : entry->name)
            key += static_cast<u8>(c);
        entry->offset = arc_file.io.read_u32_le() ^ key;
        entry->size_comp = arc_file.io.read_u32_le() ^ key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Nekopack4ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->already_unpacked)
        return nullptr;

    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp - 4);
    auto size_orig = arc_file.io.read_u32_le();

    u8 key = (entry->size_comp >> 3) + 0x22;
    auto output_ptr = data.get<u8>();
    auto output_end = data.end<const u8>();
    while (output_ptr < output_end && key)
    {
        *output_ptr++ ^= key;
        key <<= 3;
    }
    data = util::pack::zlib_inflate(data);

    return std::make_unique<File>(entry->name, data);
}

void Nekopack4ArchiveDecoder::preprocess(
    File &arc_file, fmt::ArchiveMeta &meta, const FileSaver &saver) const
{
    // apply image masks to original sprites
    std::map<std::string, ArchiveEntryImpl*> mask_entries, sprite_entries;
    for (auto &entry : meta.entries)
    {
        auto fn = entry->name.substr(0, entry->name.find_first_of('.'));
        if (entry->name.find("alp") != std::string::npos)
            mask_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
        else if (entry->name.find("bmp") != std::string::npos)
            sprite_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
    }

    fmt::microsoft::BmpImageDecoder bmp_image_decoder;
    for (auto it : sprite_entries)
    {
        try
        {
            auto sprite_entry = it.second;
            auto mask_entry = mask_entries.at(it.first);
            auto sprite_file = read_file(arc_file, meta, *sprite_entry);
            auto mask_file = read_file(arc_file, meta, *mask_entry);
            mask_file->io.seek(0);
            auto sprite = bmp_image_decoder.decode(*sprite_file);
            pix::Grid mask(
                sprite.width(),
                sprite.height(),
                mask_file->io,
                pix::Format::Gray8);
            sprite.apply_alpha_from_mask(mask);
            sprite_entry->already_unpacked = true;
            mask_entry->already_unpacked = true;
            saver.save(util::file_from_grid(sprite, sprite_entry->name));
        }
        catch (...)
        {
            continue;
        }
    }
}

static auto dummy = fmt::register_fmt<Nekopack4ArchiveDecoder>(
    "nekopack/nekopack4");
