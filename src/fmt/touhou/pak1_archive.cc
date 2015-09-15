// PAK1 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - [Tasofro & Team Shanghai Alice] [041230] TH07.5 - Immaterial and Missing
//   Power

#include "fmt/touhou/pak1_archive.h"
#include "err.h"
#include "fmt/touhou/pak1_image_archive.h"
#include "fmt/touhou/pak1_sound_archive.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static void decrypt(bstr &buffer, u8 a, u8 b, u8 delta)
{
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);

    if (file->name.find("musicroom.dat") != std::string::npos)
    {
        decrypt(data, 0x5C, 0x5A, 0x3D);
        file->change_extension(".txt");
    }
    else if (file->name.find(".sce") != std::string::npos)
    {
        decrypt(data, 0x63, 0x62, 0x42);
        file->change_extension(".txt");
    }
    else if (file->name.find("cardlist.dat") != std::string::npos)
    {
        decrypt(data, 0x60, 0x61, 0x41);
        file->change_extension(".txt");
    }

    file->io.write(data);
    return file;
}

static std::unique_ptr<io::BufferedIO> read_raw_table(
    io::IO &arc_io, size_t file_count)
{
    size_t table_size = file_count * 0x6C;
    if (table_size > arc_io.size() - arc_io.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (0x64 + 4 + 4))
        throw err::RecognitionError();
    auto buffer = arc_io.read(table_size);
    decrypt(buffer, 0x64, 0x64, 0x4D);
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
        entry->name = table_io->read_to_zero(0x64).str();
        entry->size = table_io->read_u32_le();
        entry->offset = table_io->read_u32_le();
        if (entry->offset + entry->size > arc_io.size())
            throw err::BadDataOffsetError();
        table.push_back(std::move(entry));
    }
    return table;
}

struct Pak1Archive::Priv final
{
    Pak1ImageArchive image_archive;
    Pak1SoundArchive sound_archive;
};

Pak1Archive::Pak1Archive() : p(new Priv)
{
    add_transformer(&p->image_archive);
    add_transformer(&p->sound_archive);
}

Pak1Archive::~Pak1Archive()
{
}

bool Pak1Archive::is_recognized_internal(File &arc_file) const
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

void Pak1Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<Pak1Archive>("th/pak1");
