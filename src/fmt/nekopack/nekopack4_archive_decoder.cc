#include "fmt/nekopack/nekopack4_archive_decoder.h"
#include <map>
#include "err.h"
#include "fmt/microsoft/bmp_image_decoder.h"
#include "io/memory_stream.h"
#include "util/file_from_image.h"
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

bool Nekopack4ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Nekopack4ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto table_size = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto name_size = input_file.stream.read_u32_le();
        if (!name_size)
            break;

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->already_unpacked = false;
        entry->path = input_file.stream.read_to_zero(name_size).str();
        u32 key = 0;
        for (const u8 &c : entry->path.str())
            key += c;
        entry->offset = input_file.stream.read_u32_le() ^ key;
        entry->size_comp = input_file.stream.read_u32_le() ^ key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Nekopack4ArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->already_unpacked)
        return nullptr;

    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp - 4);
    auto size_orig = input_file.stream.read_u32_le();

    u8 key = (entry->size_comp >> 3) + 0x22;
    auto output_ptr = data.get<u8>();
    auto output_end = data.end<const u8>();
    while (output_ptr < output_end && key)
    {
        *output_ptr++ ^= key;
        key <<= 3;
    }
    data = util::pack::zlib_inflate(data);

    return std::make_unique<io::File>(entry->path, data);
}

void Nekopack4ArchiveDecoder::preprocess(
    io::File &input_file,
    fmt::ArchiveMeta &meta,
    const FileSaver &file_saver) const
{
    // apply image masks to original sprites
    std::map<std::string, ArchiveEntryImpl*> mask_entries, sprite_entries;
    for (auto &entry : meta.entries)
    {
        auto fn = entry->path.stem();
        if (entry->path.has_extension("alp"))
            mask_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
        else if (entry->path.has_extension("bmp"))
            sprite_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
    }

    fmt::microsoft::BmpImageDecoder bmp_image_decoder;
    for (auto it : sprite_entries)
    {
        try
        {
            auto sprite_entry = it.second;
            auto mask_entry = mask_entries.at(it.first);
            auto sprite_file = read_file(input_file, meta, *sprite_entry);
            auto mask_file = read_file(input_file, meta, *mask_entry);
            mask_file->stream.seek(0);
            auto sprite = bmp_image_decoder.decode(*sprite_file);
            res::Image mask(
                sprite.width(),
                sprite.height(),
                mask_file->stream,
                res::PixelFormat::Gray8);
            sprite.apply_mask(mask);
            sprite_entry->already_unpacked = true;
            mask_entry->already_unpacked = true;
            file_saver.save(util::file_from_image(sprite, sprite_entry->path));
        }
        catch (...)
        {
            continue;
        }
    }
}

static auto dummy = fmt::register_fmt<Nekopack4ArchiveDecoder>(
    "nekopack/nekopack4");
