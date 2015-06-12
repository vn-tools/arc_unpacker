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
#include "formats/touhou/pak2_archive.h"
#include "formats/touhou/pak2_image_converter.h"
#include "formats/touhou/pak2_sound_converter.h"
#include "io/buffered_io.h"
#include "io/file_io.h"
#include "util/colors.h"
#include "util/encoding.h"
#include "util/mt.h"
using namespace Formats::Touhou;

namespace
{
    typedef struct
    {
        std::string name;
        u32 offset;
        u32 size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    void decrypt(IO &io, u32 mt_seed, u8 a, u8 b, u8 delta)
    {
        size_t size = io.size();
        std::unique_ptr<char[]> buffer(new char[size]);
        io.seek(0);
        io.read(buffer.get(), size);
        mt_init_genrand(mt_seed);
        for (size_t i = 0; i < size; i++)
        {
            buffer[i] ^= mt_genrand_int32();
            buffer[i] ^= a;
            a += b;
            b += delta;
        }
        io.seek(0);
        io.write(buffer.get(), size);
        io.seek(0);
    }

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<char[]> data(new char[table_entry.size]);
        arc_io.seek(table_entry.offset);
        arc_io.read(data.get(), table_entry.size);

        u8 key = (table_entry.offset >> 1) | 0x23;
        for (size_t i = 0; i < table_entry.size; i++)
            data[i] ^= key;

        std::unique_ptr<File> file(new File);
        file->io.write(data.get(), table_entry.size);
        file->name = table_entry.name;
        return file;
    }

    std::unique_ptr<BufferedIO> read_raw_table(IO &arc_io, size_t file_count)
    {
        size_t table_size = arc_io.read_u32_le();
        if (table_size > arc_io.size() - arc_io.tell())
            throw std::runtime_error("Not a PAK2 archive");
        std::unique_ptr<BufferedIO> table_io(new BufferedIO());
        table_io->write_from_io(arc_io, table_size);
        decrypt(*table_io, table_size + 6, 0xc5, 0x83, 0x53);
        return table_io;
    }

    Table read_table(IO &arc_io)
    {
        u16 file_count = arc_io.read_u16_le();
        if (file_count == 0 && arc_io.size() != 6)
            throw std::runtime_error("Not a PAK2 archive");
        auto table_io = read_raw_table(arc_io, file_count);
        Table table;
        table.reserve(file_count);
        for (size_t i = 0; i < file_count; i++)
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->offset = table_io->read_u32_le();
            entry->size = table_io->read_u32_le();
            entry->name = convert_encoding(
                table_io->read(table_io->read_u8()), "cp932", "utf-8");
            if (entry->offset + entry->size > arc_io.size())
                throw std::runtime_error("Bad offset to file");
            table.push_back(std::move(entry));
        }
        return table;
    }

    PaletteMap find_all_palettes(const std::string &arc_path)
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
                FileIO file_io(it->path(), FileIOMode::Read);
                for (auto &table_entry : read_table(file_io))
                {
                    if (table_entry->name.find(".pal") == std::string::npos)
                        continue;

                    auto pal_file = read_file(file_io, *table_entry);
                    pal_file->io.seek(1);
                    Palette palette;
                    for (size_t i = 0; i < 256; i++)
                        palette[i] = rgba5551(pal_file->io.read_u16_le());
                    palettes[table_entry->name] = palette;
                }
            }
            catch (...)
            {
                continue;
            }
        }

        return palettes;
    }
}

struct Pak2Archive::Internals
{
    Pak2ImageConverter image_converter;
    Pak2SoundConverter sound_converter;
};

Pak2Archive::Pak2Archive() : internals(new Internals)
{
    add_transformer(&internals->image_converter);
    add_transformer(&internals->sound_converter);
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
    internals->image_converter.set_palette_map(palette_map);

    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
