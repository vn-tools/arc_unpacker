// PAK1 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - Touhou 07.5 - Immaterial and Missing Power

#include "buffered_io.h"
#include "formats/touhou/pak1_archive.h"
#include "formats/touhou/pak1_image_archive.h"
#include "formats/touhou/pak1_sound_archive.h"
using namespace Formats::Touhou;

namespace
{
    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    void decrypt(IO &io, uint8_t a, uint8_t b, uint8_t delta)
    {
        size_t size = io.size();
        std::unique_ptr<char[]> buffer(new char[size]);
        io.seek(0);
        io.read(buffer.get(), size);
        for (size_t i = 0; i < size; i ++)
        {
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
        std::unique_ptr<File> file(new File);
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);
        file->name = table_entry.name;
        return file;
    }

    std::unique_ptr<BufferedIO> read_raw_table(IO &arc_io, size_t file_count)
    {
        size_t table_size = file_count * 0x6c;
        if (table_size > arc_io.size() - arc_io.tell())
            throw std::runtime_error("Not a PAK1 archive");
        std::unique_ptr<BufferedIO> table_io(new BufferedIO());
        table_io->write_from_io(arc_io, table_size);
        decrypt(*table_io, 0x64, 0x64, 0x4d);
        return table_io;
    }

    Table read_table(IO &arc_io)
    {
        uint16_t file_count = arc_io.read_u16_le();
        if (file_count == 0 && arc_io.size() != 6)
            throw std::runtime_error("Not a PAK1 archive");
        auto table_io = read_raw_table(arc_io, file_count);
        Table table;
        table.reserve(file_count);
        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->name = table_io->read_until_zero(0x64);
            entry->size = table_io->read_u32_le();
            entry->offset = table_io->read_u32_le();
            if (entry->offset + entry->size > arc_io.size())
                throw std::runtime_error("Bad offset to file");
            table.push_back(std::move(entry));
        }
        return table;
    }
}

void Pak1Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Pak1SoundArchive sound_archive;
    Pak1ImageArchive image_archive;

    auto table = read_table(arc_file.io);
    for (auto &table_entry : table)
    {
        FileSaverMemory file_saver_memory;
        bool file_is_another_archive = false;
        auto file = read_file(arc_file.io, *table_entry);

        //decode the file
        if (file->name.find("musicroom.dat") != std::string::npos)
        {
            decrypt(file->io, 0x5c, 0x5a, 0x3d);
            file->change_extension(".txt");
        }
        else if (file->name.find(".sce") != std::string::npos)
        {
            decrypt(file->io, 0x63, 0x62, 0x42);
            file->change_extension(".txt");
        }
        else if (file->name.find("cardlist.dat") != std::string::npos)
        {
            decrypt(file->io, 0x60, 0x61, 0x41);
            file->change_extension(".txt");
        }
        else if (file->name.find(".dat") != std::string::npos)
        {
            if (file->name.find("wave") != std::string::npos)
            {
                file_is_another_archive
                    |= sound_archive.try_unpack(*file, file_saver_memory);
            }
            else
            {
                file_is_another_archive
                    |= image_archive.try_unpack(*file, file_saver_memory);
            }
        }

        for (auto &subfile : file_saver_memory.get_saved())
        {
            subfile->name = file->name + "/" + subfile->name;
            file_saver.save(std::move(subfile));
        }

        if (!file_is_another_archive)
            file_saver.save(std::move(file));
    }
}
