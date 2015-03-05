// PAK archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - Touhou 07.5 - Immaterial and Missing Power

#include "buffered_io.h"
#include "formats/image.h"
#include "formats/sound.h"
#include "formats/touhou/pak_archive.h"
#include "util/colors.h"
#include "util/itos.h"
using namespace Formats::Touhou;

namespace
{
    class SoundArchive : public Archive
    {
    public:
        void unpack_internal(File &, FileSaver &) const override;
    };

    std::unique_ptr<File> read_sound(IO &arc_io, size_t index)
    {
        size_t size = arc_io.read_u32_le();
        auto format = arc_io.read_u16_le();
        auto channel_count = arc_io.read_u16_le();
        auto sample_rate = arc_io.read_u32_le();
        auto byte_rate = arc_io.read_u32_le();
        auto block_align = arc_io.read_u16_le();
        auto bits_per_sample = arc_io.read_u16_le();
        arc_io.skip(2);

        auto sound = Sound::from_samples(
            channel_count,
            bits_per_sample / 8,
            sample_rate,
            arc_io.read(size));

        std::unique_ptr<File> file(new File);
        file->name = itos(index);
        sound->update_file(*file);
        return file;
    }

    void SoundArchive::unpack_internal(
        File &arc_file, FileSaver &file_saver) const
    {
        size_t file_count = arc_file.io.read_u32_le();
        for (size_t i = 0; i < file_count; i ++)
        {
            if (!arc_file.io.read_u8())
                continue;

            file_saver.save(read_sound(arc_file.io, i));
        }
    }
}


namespace
{
    class ImageArchive : public Archive
    {
    public:
        void unpack_internal(File &, FileSaver &) const override;
    };

    std::unique_ptr<File> read_image(
        IO &arc_io, size_t index, uint32_t *palette)
    {
        auto image_width = arc_io.read_u32_le();
        auto image_height = arc_io.read_u32_le();
        arc_io.skip(4);
        auto channels = arc_io.read_u8();
        size_t source_size = arc_io.read_u32_le();
        size_t target_size = image_width * image_height * 4;

        BufferedIO target_io;
        target_io.reserve(target_size);
        BufferedIO source_io;
        source_io.write_from_io(arc_io, source_size);
        source_io.seek(0);

        while (source_io.tell() < source_io.size())
        {
            size_t repeat;
            uint32_t rgba;

            switch (channels)
            {
                case 32:
                    repeat = source_io.read_u32_le();
                    rgba = source_io.read_u32_le();
                    break;

                case 24:
                    repeat = source_io.read_u32_le();
                    rgba = source_io.read_u8()
                        | (source_io.read_u8() << 8)
                        | (source_io.read_u8() << 16)
                        | 0xff000000;
                    source_io.skip(1);
                    break;

                case 16:
                    repeat = source_io.read_u16_le();
                    rgba = rgba5551(source_io.read_u16_le());
                    break;

                case 8:
                    repeat = source_io.read_u8();
                    rgba = palette[static_cast<size_t>(source_io.read_u8())];
                    break;

                default:
                    throw std::runtime_error("Unsupported channel count");
            }

            while (repeat --)
                target_io.write_u32_le(rgba);
        }

        std::unique_ptr<File> file(new File);
        file->name = itos(index, 4);
        target_io.seek(0);
        auto image = Image::from_pixels(
            image_width,
            image_height,
            target_io.read(target_io.size()),
            IMAGE_PIXEL_FORMAT_BGRA);
        image->update_file(*file);
        return file;
    }

    void ImageArchive::unpack_internal(
        File &arc_file, FileSaver &file_saver) const
    {
        auto palette_size = arc_file.io.read_u8();
        std::unique_ptr<uint32_t[]> palette(new uint32_t[256 * palette_size]);
        for (size_t p = 0; p < palette_size; p ++)
            for (size_t i = 0; i < 256; i ++)
                palette[p * 256 + i] = rgba5551(arc_file.io.read_u16_le());

        size_t i = 0;
        while (arc_file.io.tell() < arc_file.io.size())
            file_saver.save(read_image(arc_file.io, i ++, palette.get()));
    }
}


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
        std::unique_ptr<BufferedIO> table_io(new BufferedIO());
        table_io->write_from_io(arc_io, table_size);
        decrypt(*table_io, 0x64, 0x64, 0x4d);
        return table_io;
    }

    Table read_table(IO &arc_io)
    {
        uint16_t file_count = arc_io.read_u16_le();
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

void PakArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    SoundArchive sound_archive;
    ImageArchive image_archive;

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
                if (sound_archive.try_unpack(*file, file_saver_memory))
                    file_is_another_archive = true;
            }
            else
            {
                if (image_archive.try_unpack(*file, file_saver_memory))
                    file_is_another_archive = true;
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
