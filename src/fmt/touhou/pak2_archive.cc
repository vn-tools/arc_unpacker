// PAK2 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - [Team Shanghai Alice] [080525] TH10.5 - Scarlet Weather Rhapsody
// - [Team Shanghai Alice] [090815] TH12.3 - Unthinkable Natural Law

#include <boost/filesystem.hpp>
#include "err.h"
#include "fmt/touhou/pak2_archive.h"
#include "fmt/touhou/pak2_image_converter.h"
#include "fmt/touhou/pak2_sound_converter.h"
#include "io/buffered_io.h"
#include "io/file_io.h"
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

static void decrypt(bstr &buffer, u32 mt_seed, u8 a, u8 b, u8 delta)
{
    util::mt::init_genrand(mt_seed);
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= util::mt::genrand_int32();
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);

    u8 key = (entry.offset >> 1) | 0x23;
    for (auto i : util::range(entry.size))
        data[i] ^= key;

    std::unique_ptr<File> file(new File);
    file->io.write(data);
    file->name = entry.name;
    return file;
}

static std::unique_ptr<io::BufferedIO> read_raw_table(
    io::IO &arc_io, size_t file_count)
{
    size_t table_size = arc_io.read_u32_le();
    if (table_size > arc_io.size() - arc_io.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (4 + 4 + 256 + 1))
        throw err::RecognitionError();
    auto buffer = arc_io.read(table_size);
    decrypt(buffer, table_size + 6, 0xC5, 0x83, 0x53);
    return std::unique_ptr<io::BufferedIO>(new io::BufferedIO(buffer));
}

static Table read_table(io::IO &arc_io)
{
    u16 file_count = arc_io.read_u16_le();
    if (file_count == 0 && arc_io.size() != 6)
        throw err::RecognitionError();
    auto table_io = read_raw_table(arc_io, file_count);
    Table table;
    table.reserve(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = table_io->read_u32_le();
        entry->size = table_io->read_u32_le();
        auto name_size = table_io->read_u8();
        entry->name = util::sjis_to_utf8(table_io->read(name_size)).str();
        if (entry->offset + entry->size > arc_io.size())
            throw err::BadDataOffsetError();
        table.push_back(std::move(entry));
    }
    return table;
}

static void register_palettes(
    const std::string &arc_path, Pak2ImageConverter &image_converter)
{
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
                pal_file->io.seek(0);
                image_converter.add_palette(
                    entry->name, pal_file->io.read_to_eof());
            }
        }
        catch (...)
        {
            continue;
        }
    }
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
    register_palettes(arc_file.name, p->image_converter);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<Pak2Archive>("th/pak2");
