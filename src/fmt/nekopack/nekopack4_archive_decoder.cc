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
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size_comp;
        bool already_unpacked;
    };

    using Table = std::vector<TableEntry>;
}

static Table read_table(io::IO &arc_io)
{
    auto table_size = arc_io.read_u32_le();
    Table table;
    while (true)
    {
        auto name_size = arc_io.read_u32_le();
        if (!name_size)
            break;

        TableEntry entry { };
        entry.name = arc_io.read_to_zero(name_size).str();
        u32 key = 0;
        for (auto &c : entry.name)
            key += static_cast<u8>(c);
        entry.offset = arc_io.read_u32_le() ^ key;
        entry.size_comp = arc_io.read_u32_le() ^ key;
        table.push_back(entry);
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_comp - 4);
    auto size_orig = arc_io.read_u32_le();

    u8 key = (entry.size_comp >> 3) + 0x22;
    auto output_ptr = data.get<u8>();
    auto output_end = data.end<const u8>();
    while (output_ptr < output_end && key)
    {
        *output_ptr++ ^= key;
        key <<= 3;
    }
    data = util::pack::zlib_inflate(data);

    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    file->io.write(data);
    return file;
}

bool Nekopack4ArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void Nekopack4ArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());
    auto table = read_table(arc_file.io);

    if (nested_decoding_enabled)
    {
        // apply image masks to original sprites
        std::map<std::string, TableEntry*> mask_entries, sprite_entries;
        for (auto i : util::range(table.size()))
        {
            auto fn = table[i].name.substr(0, table[i].name.find_first_of('.'));
            if (table[i].name.find("alp") != std::string::npos)
                mask_entries[fn] = &table[i];
            else if (table[i].name.find("bmp") != std::string::npos)
                sprite_entries[fn] = &table[i];
        }

        fmt::microsoft::BmpImageDecoder bmp_image_decoder;
        for (auto it : sprite_entries)
        {
            try
            {
                auto sprite_entry = it.second;
                auto mask_entry = mask_entries.at(it.first);
                auto sprite_file = read_file(arc_file.io, *sprite_entry);
                auto mask_file = read_file(arc_file.io, *mask_entry);
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

    for (auto &entry : table)
        if (!entry.already_unpacked)
            saver.save(read_file(arc_file.io, entry));
}

static auto dummy = fmt::Registry::add<Nekopack4ArchiveDecoder>(
    "nekopack/nekopack4");
