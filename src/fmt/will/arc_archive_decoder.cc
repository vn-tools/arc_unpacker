#include "fmt/will/arc_archive_decoder.h"
#include <map>
#include "err.h"
#include "fmt/png/png_image_decoder.h"
#include "fmt/will/wipf_archive_decoder.h"
#include "util/file_from_grid.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::will;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size;
        bool already_unpacked;
    };

    struct DirectoryEntry final
    {
        std::string extension;
        u32 offset;
        u32 file_count;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_inner_table(
    io::IO &arc_io,
    const std::vector<DirectoryEntry> &directories,
    size_t name_size)
{
    auto min_offset = 4 + directories.size() * 12;
    for (auto &directory : directories)
        min_offset += directory.file_count * (name_size + 8);

    Table table;
    for (auto &directory : directories)
    {
        arc_io.seek(directory.offset);
        for (auto i : util::range(directory.file_count))
        {
            std::unique_ptr<TableEntry> entry(new TableEntry());
            auto name = arc_io.read_to_zero(name_size).str();
            entry->name = name + "." + directory.extension;
            entry->size = arc_io.read_u32_le();
            entry->offset = arc_io.read_u32_le();

            if (!entry->name.size())
                throw err::CorruptDataError("Empty file name");
            if (entry->offset < min_offset)
                throw err::BadDataOffsetError();
            if (entry->offset + entry->size > arc_io.size())
                throw err::BadDataOffsetError();

            table.push_back(std::move(entry));
        }
    }
    return table;
}

static Table read_table(io::IO &arc_io)
{
    std::vector<DirectoryEntry> directories(arc_io.read_u32_le());
    for (auto i : util::range(directories.size()))
    {
        directories[i].extension = arc_io.read_to_zero(4).str();
        directories[i].file_count = arc_io.read_u32_le();
        directories[i].offset = arc_io.read_u32_le();
    }

    for (auto name_size : {9, 13})
    {
        try
        {
            return read_inner_table(arc_io, directories, name_size);
        }
        catch (...)
        {
            continue;
        }
    }

    throw err::CorruptDataError("Failed to read file table");
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);

    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    if (file->has_extension("wsc") || file->has_extension("scr"))
        for (auto &c : data)
            c = (c >> 2) | (c << 6);

    file->io.write(data);
    return file;
}

struct ArcArchiveDecoder::Priv final
{
    WipfArchiveDecoder wipf_archive_decoder;
};

ArcArchiveDecoder::ArcArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->wipf_archive_decoder);
}

ArcArchiveDecoder::~ArcArchiveDecoder()
{
}

bool ArcArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return read_table(arc_file.io).size() > 0;
}

void ArcArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto table = read_table(arc_file.io);

    // apply image masks to original sprites
    std::map<std::string, TableEntry*> mask_entries, sprite_entries;
    for (auto &entry : table)
    {
        auto fn = entry->name.substr(0, entry->name.find_first_of('.'));
        if (entry->name.find("MSK") != std::string::npos)
            mask_entries[fn] = entry.get();
        else if (entry->name.find("WIP") != std::string::npos)
            sprite_entries[fn] = entry.get();
    }

    fmt::png::PngImageDecoder png_decoder;
    for (auto it : sprite_entries)
    {
        try
        {
            auto sprite_entry = it.second;
            auto mask_entry = mask_entries.at(it.first);
            auto sprites = p->wipf_archive_decoder.unpack(
                *read_file(arc_file.io, *sprite_entry), false);
            auto masks = p->wipf_archive_decoder.unpack(
                *read_file(arc_file.io, *mask_entry), false);

            std::vector<pix::Grid> images;
            for (auto i : util::range(sprites.size()))
            {
                auto img = png_decoder.decode(*sprites[i]);
                img.apply_alpha_from_mask(png_decoder.decode(*masks.at(i)));
                images.push_back(img);
            }

            sprite_entry->already_unpacked = true;
            mask_entry->already_unpacked = true;
            for (auto &img : images)
                saver.save(util::file_from_grid(img, sprite_entry->name));
        }
        catch (...)
        {
            continue;
        }
    }

    for (auto &entry : table)
        if (!entry->already_unpacked)
            saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<ArcArchiveDecoder>("will/arc");
