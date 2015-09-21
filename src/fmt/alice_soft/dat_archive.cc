#include "fmt/alice_soft/dat_archive.h"
#include <boost/algorithm/string.hpp>
#include "fmt/alice_soft/vsp_converter.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

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

static Table read_table(io::IO &arc_io, std::string arc_name)
{
    boost::algorithm::to_lower(arc_name);

    auto header_size = (arc_io.read_u16_le() - 1) * 256;
    Table table;

    size_t last_offset = 0;
    bool finished = false;
    for (auto i : util::range((header_size / 2) - 1))
    {
        size_t offset = arc_io.read_u16_le();
        if (offset == 0)
        {
            finished = true;
            continue;
        }
        offset = (offset - 1) * 256;

        if (offset < last_offset)
            throw err::CorruptDataError("Expected offsets to be sorted");
        if (finished)
            throw err::CorruptDataError("Expected remaining offsets to be 0");
        if (offset > arc_io.size())
            throw err::BadDataOffsetError();
        if (offset == arc_io.size())
            continue;

        //necessary for VSP recognition
        std::string ext = "dat";
        if (arc_name.find("cg") != std::string::npos)
            ext = "vsp";
        else if (arc_name.find("dis") != std::string::npos)
            ext = "sco";
        else if (arc_name.find("mus") != std::string::npos)
            ext = "mus";
        else if (arc_name.find("map") != std::string::npos)
            ext = "map";

        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = offset;
        entry->name = util::format("%03d.%s", i, ext.c_str());
        table.push_back(std::move(entry));

        last_offset = offset;
    }

    if (!table.size())
        throw err::CorruptDataError("File table cannot be empty");

    for (size_t i : util::range(table.size()))
    {
        size_t next_offset = i + 1 < table.size()
            ? table[i + 1]->offset
            : arc_io.size();
        table[i]->size = next_offset - table[i]->offset;
    }

    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    file->name = entry.name;
    return file;
}

bool DatArchive::is_recognized_internal(File &arc_file) const
{
    try
    {
        read_table(arc_file.io, arc_file.name);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

struct DatArchive::Priv final
{
    VspConverter vsp_converter;
};

DatArchive::DatArchive() : p(new Priv)
{
    add_transformer(&p->vsp_converter);
}

DatArchive::~DatArchive()
{
}

void DatArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io, arc_file.name);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<DatArchive>("alice/dat");
