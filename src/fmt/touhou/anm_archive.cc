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

#include <algorithm>
#include <map>
#include "fmt/touhou/anm_archive.h"
#include "util/colors.h"
#include "util/image.h"
#include "util/itos.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::touhou;

namespace
{
    typedef struct
    {
        size_t width;
        size_t height;
        size_t x, y;
        size_t format;
        std::string name;
        int version;
        size_t texture_offset;
        bool has_data;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string texture_magic = "THTX"_s;

static std::string read_name(io::IO &file_io, size_t offset)
{
    std::string name;
    file_io.peek(offset, [&]() { name = file_io.read_until_zero(); });
    return name;
}

static size_t read_old_entry(
    TableEntry &entry, io::IO &file_io, size_t base_offset)
{
    file_io.skip(4); //sprite count
    file_io.skip(4); //script count
    file_io.skip(4); //zero

    entry.width = file_io.read_u32_le();
    entry.height = file_io.read_u32_le();
    entry.format = file_io.read_u32_le();
    entry.x = file_io.read_u16_le();
    entry.y = file_io.read_u16_le();
    //file_io.skip(4);

    size_t name_offset1 = base_offset + file_io.read_u32_le();
    file_io.skip(4);
    size_t name_offset2 = base_offset + file_io.read_u32_le();
    entry.name = read_name(file_io, name_offset1);

    entry.version = file_io.read_u32_le();
    file_io.skip(4);
    entry.texture_offset = base_offset + file_io.read_u32_le();
    entry.has_data = file_io.read_u32_le() > 0;

    return base_offset + file_io.read_u32_le();
}

static size_t read_new_entry(
    TableEntry &entry, io::IO &file_io, size_t base_offset)
{
    entry.version = file_io.read_u32_le();
    file_io.skip(2); //sprite count
    file_io.skip(2); //script count
    file_io.skip(2); //zero

    entry.width = file_io.read_u16_le();
    entry.height = file_io.read_u16_le();
    entry.format = file_io.read_u16_le();
    size_t name_offset = base_offset + file_io.read_u32_le();
    entry.name = read_name(file_io, name_offset);
    entry.x = file_io.read_u16_le();
    entry.y = file_io.read_u16_le();
    file_io.skip(4);

    entry.texture_offset = base_offset + file_io.read_u32_le();
    entry.has_data = file_io.read_u16_le() > 0;
    file_io.skip(2);

    return base_offset + file_io.read_u32_le();
}

static Table read_table(io::IO &file_io)
{
    Table table;
    u32 base_offset = 0;
    while (true)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        file_io.seek(base_offset);
        file_io.skip(8);
        bool use_old = file_io.read_u32_le() == 0;

        file_io.seek(base_offset);
        size_t next_offset = use_old
            ? read_old_entry(*entry, file_io, base_offset)
            : read_new_entry(*entry, file_io, base_offset);

        table.push_back(std::move(entry));
        if (next_offset == base_offset)
            break;
        base_offset = next_offset;
    }
    return table;
}

static void write_pixels(
    io::IO &file_io, TableEntry &entry, u32 *pixel_data, size_t stride)
{
    if (!entry.has_data)
        return;

    file_io.seek(entry.texture_offset);
    if (file_io.read(texture_magic.size()) != texture_magic)
        throw std::runtime_error("Corrupt texture data");
    file_io.skip(2);
    int format = file_io.read_u16_le();
    size_t width = file_io.read_u16_le();
    size_t height = file_io.read_u16_le();
    size_t data_size = file_io.read_u32_le();

    u32 *guardian = &pixel_data[stride * height];

    for (size_t y = 0; y < height; y++)
    {
        size_t shift = (y + entry.y) * stride + entry.x;
        u32 *pixel_ptr = &pixel_data[shift];
        for (size_t x = 0; x < width; x++)
        {
            if (pixel_ptr >= guardian)
                return;
            switch (format)
            {
                case 1:
                    *pixel_ptr++ = file_io.read_u32_le();
                    break;

                case 3:
                    *pixel_ptr++ = util::color::rgb565(file_io.read_u16_le());
                    break;

                case 5:
                    *pixel_ptr++ = util::color::rgba4444(file_io.read_u16_le());
                    break;

                case 7:
                    *pixel_ptr++ = util::color::rgba_gray(file_io.read_u8());
                    break;

                default:
                    throw std::runtime_error(
                        "Unknown color format: " + util::itos(format));
            }
        }
    }
}

static std::unique_ptr<File> read_texture(io::IO &file_io, Table &entries)
{
    size_t width = 0;
    size_t height = 0;
    for (auto &entry : entries)
    {
        if (!entry->has_data)
            continue;
        file_io.peek(entry->texture_offset, [&]()
        {
            file_io.skip(texture_magic.size());
            file_io.skip(2);
            file_io.skip(2);
            size_t chunk_width = file_io.read_u16_le();
            size_t chunk_height = file_io.read_u16_le();
            width = std::max(width, entry->x + chunk_width);
            height = std::max(height, entry->y + chunk_height);
        });
    }
    if (!width || !height)
        return nullptr;

    size_t pixel_data_size = width * height;
    std::unique_ptr<u32[]> pixel_data(new u32[pixel_data_size]);
    for (size_t i = 0; i < pixel_data_size; i++)
        pixel_data[i] = 0;
    for (auto &entry : entries)
        write_pixels(file_io, *entry, pixel_data.get(), width);

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        width,
        height,
        std::string(
            reinterpret_cast<char*>(pixel_data.get()), pixel_data_size * 4),
        util::PixelFormat::BGRA);
    return image->create_file(entries[0]->name);
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

    std::map<std::string, Table> table_map;
    for (auto &entry : table)
        table_map[entry->name].push_back(std::move(entry));

    for (auto &kv : table_map)
    {
        auto &name = kv.first;
        auto &entries = kv.second;

        // Ignore both the scripts and sprites and extract raw texture data.
        auto file = read_texture(arc_file.io, entries);
        if (file != nullptr)
            file_saver.save(std::move(file));
    }
}
