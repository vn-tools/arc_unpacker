#include "fmt/entis/noa_archive.h"
#include "fmt/entis/common/sections.h"
#include "fmt/entis/eri_converter.h"
#include "fmt/entis/mio_converter.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x04\x00\x02\x00\x00\x00\x00"_b;
static const bstr magic3 = "ERISA-Archive file"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t offset;
        size_t size;
        bool encrypted;
        bstr extra;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &io, std::string root = "")
{
    Table table;
    common::SectionReader section_reader(io);
    for (auto &section : section_reader.get_sections("DirEntry"))
    {
        io.seek(section.offset);
        auto entry_count = io.read_u32_le();
        for (auto i : util::range(entry_count))
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->size = io.read_u64_le();
            auto flags = io.read_u32_le();
            entry->encrypted = io.read_u32_le() > 0;
            entry->offset = section.offset + io.read_u64_le();
            io.skip(8);

            auto extra_size = io.read_u32_le();
            if (flags & 0x70)
                entry->extra = io.read(extra_size);

            entry->name = io.read(io.read_u32_le()).str();
            if (root != "")
                entry->name = root + "/" + entry->name;

            if (flags == 0x10)
            {
                Table sub_table;
                io.peek(entry->offset, [&]()
                {
                    sub_table = read_table(io, entry->name);
                });
                for (auto &sub_entry : sub_table)
                    table.push_back(std::move(sub_entry));
            }
            else if (flags == 0x20 || flags == 0x40)
            {
            }
            else
            {
                table.push_back(std::move(entry));
            }
        }
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    return file;
}

struct NoaArchive::Priv final
{
    EriConverter eri_converter;
    MioConverter mio_converter;
};

NoaArchive::NoaArchive() : p(new Priv)
{
    add_transformer(&p->eri_converter);
    add_transformer(&p->mio_converter);
}

NoaArchive::~NoaArchive()
{
}

bool NoaArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic1.size()) == magic1
        && arc_file.io.read(magic2.size()) == magic2
        && arc_file.io.read(magic3.size()) == magic3;
}

void NoaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.seek(0x40);
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        if (entry->encrypted)
        {
            Log.warn(util::format(
                "%s is encrypted, but encrypted files are not supported\n",
                entry->name.c_str()));
        }
        file_saver.save(read_file(arc_file.io, *entry));
    }
}

static auto dummy = fmt::Registry::add<NoaArchive>("entis/noa");
