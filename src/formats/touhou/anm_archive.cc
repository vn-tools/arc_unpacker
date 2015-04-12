// ANM file
//
// Company:   Team Shanghai Anime
// Engine:    -
// Extension: .anm
//
// Known games:
// - Touhou 06 - The Embodiment of Scarlet Devil
// - Touhou 07 - Perfect Cherry Blossom
// - Touhou 08 - Imperishable Night
// - Touhou 09 - Phantasmagoria of Flower View
// - Touhou 09.5 - Shoot the Bullet
// - Touhou 10 - Mountain of Faith
// - Touhou 11 - Subterranean Animism
// - Touhou 12 - Undefined Fantastic Object
// - Touhou 12.5 - Double Spoiler
// - Touhou 12.8 - Fairy Wars
// - Touhou 13 - Ten Desires
// - Touhou 14 - Double Dealing Character

#include "formats/touhou/anm_archive.h"
#include "util/colors.h"
#include "util/image.h"
#include "util/itos.h"
using namespace Formats::Touhou;

namespace
{
    const std::string texture_magic("THTX", 4);

    typedef struct
    {
        size_t width;
        size_t height;
        size_t format;
        std::string name;
        int version;
        size_t texture_offset;
        bool has_data;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::string read_name(IO &file_io, size_t offset)
    {
        std::string name;
        file_io.peek(offset, [&]() { name = file_io.read_until_zero(); });
        return name;
    }

    size_t read_old_entry(
        TableEntry &table_entry, IO &file_io, size_t start_offset)
    {
        file_io.skip(4); //sprite count
        file_io.skip(4); //script count
        file_io.skip(4); //zero

        table_entry.width = file_io.read_u32_le();
        table_entry.height = file_io.read_u32_le();
        table_entry.format = file_io.read_u32_le();
        file_io.skip(4);

        size_t name_offset1 = start_offset + file_io.read_u32_le();
        file_io.skip(4);
        size_t name_offset2 = start_offset + file_io.read_u32_le();
        table_entry.name = read_name(file_io, name_offset1);

        table_entry.version = file_io.read_u32_le();
        file_io.skip(4);
        table_entry.texture_offset = start_offset + file_io.read_u32_le();
        table_entry.has_data = file_io.read_u32_le() > 0;

        return start_offset + file_io.read_u32_le();
    }

    size_t read_new_entry(
        TableEntry &table_entry, IO &file_io, size_t start_offset)
    {
        table_entry.version = file_io.read_u32_le();
        file_io.skip(2); //sprite count
        file_io.skip(2); //script count
        file_io.skip(2); //zero

        table_entry.width = file_io.read_u16_le();
        table_entry.height = file_io.read_u16_le();
        table_entry.format = file_io.read_u16_le();
        size_t name_offset = start_offset + file_io.read_u32_le();
        table_entry.name = read_name(file_io, name_offset);
        file_io.skip(2 * 2 + 4);

        table_entry.texture_offset = start_offset + file_io.read_u32_le();
        table_entry.has_data = file_io.read_u16_le() > 0;
        file_io.skip(2);

        return start_offset + file_io.read_u32_le();
    }

    Table read_table(IO &file_io)
    {
        Table table;
        uint32_t start_offset = 0;
        while (true)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);

            file_io.seek(start_offset);
            file_io.skip(8);
            bool use_old = file_io.read_u32_le() == 0;

            file_io.seek(start_offset);
            size_t next_offset = use_old
                ? read_old_entry(*table_entry, file_io, start_offset)
                : read_new_entry(*table_entry, file_io, start_offset);

            table.push_back(std::move(table_entry));
            if (next_offset == start_offset)
                break;
            start_offset = next_offset;
        }
        return table;
    }

    std::unique_ptr<File> read_texture(IO &file_io, TableEntry &table_entry)
    {
        if (!table_entry.has_data)
            return nullptr;

        file_io.seek(table_entry.texture_offset);
        if (file_io.read(texture_magic.size()) != texture_magic)
            throw std::runtime_error("Corrupt texture data");
        file_io.skip(2);
        int format = file_io.read_u16_le();
        size_t width = file_io.read_u16_le();
        size_t height = file_io.read_u16_le();
        size_t data_size = file_io.read_u32_le();

        size_t pixel_data_size = width * height * 4;
        std::unique_ptr<char[]> pixel_data(new char[pixel_data_size]);
        uint32_t *pixel_ptr = reinterpret_cast<uint32_t*>(pixel_data.get());
        for (size_t y = 0; y < height; y ++)
        {
            for (size_t x = 0; x < width; x ++)
            {
                switch (format)
                {
                    case 1:
                        *pixel_ptr ++ = file_io.read_u32_le();
                        break;

                    case 3:
                        *pixel_ptr ++ = rgb565(file_io.read_u16_le());
                        break;

                    case 5:
                        *pixel_ptr ++ = rgba4444(file_io.read_u16_le());
                        break;

                    case 7:
                        *pixel_ptr ++ = rgba_gray(file_io.read_u8());
                        break;

                    default:
                        throw std::runtime_error(
                            "Unknown color format: " + itos(format));
                }
            }
        }

        std::unique_ptr<Image> image = Image::from_pixels(
            width,
            height,
            std::string(pixel_data.get(), pixel_data_size),
            PixelFormat::BGRA);
        return image->create_file(table_entry.name);
    }
}

FileNamingStrategy AnmArchive::get_file_naming_strategy() const
{
    return FileNamingStrategy::Root;
}

bool AnmArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("anm");
}

void AnmArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &table_entry : table)
    {
        // Ignore both the scripts and sprites and extract raw texture data.
        auto file = read_texture(arc_file.io, *table_entry);
        if (file != nullptr)
            file_saver.save(std::move(file));
    }
}
