// LNK archive
//
// Company:   KID
// Engine:    -
// Extension: .dat
//
// Known games:
// - Ever 17

#include "fmt/kid/cps_converter.h"
#include "fmt/kid/decompressor.h"
#include "fmt/kid/lnk_archive.h"
#include "fmt/kid/prt_converter.h"
#include "fmt/kid/waf_converter.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kid;

static const std::string magic = "LNK\x00"_s;
static const std::string compress_magic = "lnd\x00"_s;

namespace
{
    typedef struct
    {
        std::string name;
        bool compressed;
        u32 offset;
        u32 size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

std::unique_ptr<io::IO> lnd_decompress(io::IO &input_io)
{
    if (input_io.read(compress_magic.size()) != compress_magic)
        throw std::runtime_error("Unexpected file header");
    input_io.skip(4);
    size_t size_original = input_io.read_u32_le();
    input_io.skip(4);
    return decompress(input_io, size_original);
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    u32 file_count = arc_io.read_u32_le();
    arc_io.skip(8);
    auto file_data_start = arc_io.tell() + (file_count << 5);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = arc_io.read_u32_le() + file_data_start;
        u32 tmp = arc_io.read_u32_le();
        entry->compressed = tmp & 1;
        entry->size = tmp >> 1;
        entry->name = arc_io.read_until_zero(24);
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    arc_io.seek(entry.offset);
    io::BufferedIO data_io(arc_io, entry.size);

    int key_pos = -1;
    if (file->has_extension(".wav"))
        key_pos = 0;
    else if (file->has_extension(".jpg"))
        key_pos = 0x1100;
    else if (file->has_extension(".scr"))
        key_pos = 0x1000;

    if (key_pos >= 0 && key_pos < static_cast<int>(entry.size))
    {
        u8 key = 0;
        for (u8 c : entry.name)
            key += c;

        for (size_t i = 0; i < 0x100 && key_pos + i < entry.size; i++)
        {
            data_io.buffer()[key_pos + i] -= key;
            key = key * 0x6D - 0x25;
        }
    }

    data_io.seek(0);
    if (entry.compressed)
    {
        auto decompressed_io = lnd_decompress(data_io);
        file->io.write_from_io(*decompressed_io);
    }
    else
        file->io.write_from_io(data_io);

    return file;
}

struct LnkArchive::Priv
{
    CpsConverter cps_converter;
    PrtConverter prt_converter;
    WafConverter waf_converter;
};

LnkArchive::LnkArchive() : p(new Priv)
{
    add_transformer(&p->cps_converter);
    add_transformer(&p->prt_converter);
    add_transformer(&p->waf_converter);
}

LnkArchive::~LnkArchive()
{
}

bool LnkArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void LnkArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
