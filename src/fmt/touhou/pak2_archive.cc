// PAK2 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - Touhou 10.5 - Scarlet Weather Rhapsody
// - Touhou 12.3 - Unthinkable Natural Law

#include <boost/filesystem.hpp>
#include "fmt/touhou/pak2_archive.h"
#include "fmt/touhou/pak2_image_converter.h"
#include "fmt/touhou/pak2_sound_converter.h"
#include "io/buffered_io.h"
#include "io/file_io.h"
#include "util/colors.h"
#include "util/encoding.h"
#include "util/mt.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct TableEntry
    {
        std::string name;
        u32 offset;
        u32 size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static void decrypt(io::IO &io, u32 mt_seed, u8 a, u8 b, u8 delta)
{
    size_t size = io.size();
    std::unique_ptr<char[]> buffer(new char[size]);
    io.seek(0);
    io.read(buffer.get(), size);
    util::mt::init_genrand(mt_seed);
    for (auto i : util::range(size))
    {
        buffer[i] ^= util::mt::genrand_int32();
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
    io.seek(0);
    io.write(buffer.get(), size);
    io.seek(0);
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<char[]> data(new char[entry.size]);
    arc_io.seek(entry.offset);
    arc_io.read(data.get(), entry.size);

    u8 key = (entry.offset >> 1) | 0x23;
    for (auto i : util::range(entry.size))
        data[i] ^= key;

    std::unique_ptr<File> file(new File);
    file->io.write(data.get(), entry.size);
    file->name = entry.name;
    return file;
}

static std::unique_ptr<io::BufferedIO> read_raw_table(
    io::IO &arc_io, size_t file_count)
{
    size_t table_size = arc_io.read_u32_le();
    if (table_size > arc_io.size() - arc_io.tell())
        throw std::runtime_error("Not a PAK2 archive");
    if (table_size > file_count * (4 + 4 + 256 + 1))
        throw std::runtime_error("Not a PAK2 archive");
    std::unique_ptr<io::BufferedIO> table_io(new io::BufferedIO());
    table_io->write_from_io(arc_io, table_size);
    decrypt(*table_io, table_size + 6, 0xC5, 0x83, 0x53);
    return table_io;
}

static Table read_table(io::IO &arc_io)
{
    u16 file_count = arc_io.read_u16_le();
    if (file_count == 0 && arc_io.size() != 6)
        throw std::runtime_error("Not a PAK2 archive");
    auto table_io = read_raw_table(arc_io, file_count);
    Table table;
    table.reserve(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = table_io->read_u32_le();
        entry->size = table_io->read_u32_le();
        entry->name = util::sjis_to_utf8(table_io->read(table_io->read_u8()));
        if (entry->offset + entry->size > arc_io.size())
            throw std::runtime_error("Bad offset to file");
        table.push_back(std::move(entry));
    }
    return table;
}

static PaletteMap find_all_palettes(const std::string &arc_path)
{
    PaletteMap palettes;

    auto dir = boost::filesystem::path(arc_path).parent_path();
    for (boost::filesystem::directory_iterator it(dir);
        it != boost::filesystem::directory_iterator();
        it++)
    {
        if (!boost::filesystem::is_regular_file(it->path()))
            continue;
        if (it->path().string().find(".dat") == std::string::npos)
            continue;

        try
        {
            io::FileIO file_io(it->path(), io::FileMode::Read);
            for (auto &entry : read_table(file_io))
            {
                if (entry->name.find(".pal") == std::string::npos)
                    continue;

                auto pal_file = read_file(file_io, *entry);
                pal_file->io.seek(1);
                Palette palette;
                for (auto i : util::range(256))
                {
                    palette[i] = util::color::rgba5551(
                        pal_file->io.read_u16_le());
                }
                palettes[entry->name] = palette;
            }
        }
        catch (...)
        {
            continue;
        }
    }

    return palettes;
}

struct Pak2Archive::Priv
{
    Pak2ImageConverter image_converter;
    Pak2SoundConverter sound_converter;
};

Pak2Archive::Pak2Archive() : p(new Priv)
{
    add_transformer(&p->image_converter);
    add_transformer(&p->sound_converter);
}

Pak2Archive::~Pak2Archive()
{
}

bool Pak2Archive::is_recognized_internal(File &arc_file) const
{
    try
    {
        read_table(arc_file.io);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void Pak2Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io);

    PaletteMap palette_map = find_all_palettes(arc_file.name);
    p->image_converter.set_palette_map(palette_map);

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
