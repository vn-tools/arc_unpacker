#include "fmt/rpgmaker/rgss3a_archive.h"
#include "fmt/rpgmaker/rgs/common.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "RGSSAD\x00\x03"_b;

static rgs::Table read_table(io::IO &arc_io, u32 key)
{
    rgs::Table table;

    while (arc_io.tell() < arc_io.size())
    {
        std::unique_ptr<rgs::TableEntry> entry(new rgs::TableEntry);
        entry->offset = arc_io.read_u32_le() ^ key;
        if (!entry->offset)
            break;

        entry->size = arc_io.read_u32_le() ^ key;
        entry->key = arc_io.read_u32_le() ^ key;

        size_t name_size = arc_io.read_u32_le() ^ key;
        entry->name = arc_io.read(name_size).str();
        for (auto i : util::range(name_size))
            entry->name[i] ^= key >> (i << 3);

        table.push_back(std::move(entry));
    }
    return table;
}

bool Rgss3aArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void Rgss3aArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    u32 key = arc_file.io.read_u32_le() * 9 + 3;
    auto table = read_table(arc_file.io, key);
    for (auto &entry : table)
        file_saver.save(rgs::read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<Rgss3aArchive>("rm/rgss3a");
