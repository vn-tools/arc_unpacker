// EAGLS PAK archive
//
// Company:   TechArts
// Engine:    Enhanced Adventure Game Language System
// Extension: .pak
//
// Known games:
// - Honoo no Haramase Tenkousei

#include <algorithm>
#include <boost/filesystem.hpp>
#include "fmt/eagls/pak_archive.h"
#include "io/buffered_io.h"
#include "io/file_io.h"
#include "util/crypt/lcg.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::eagls;

static const bstr key = "1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik,9ol.0p;/-@:^[]"_b;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t offset;
        size_t size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static std::string get_path_to_index(const std::string &path_to_data)
{
    boost::filesystem::path index_path(path_to_data);
    index_path.replace_extension("idx");
    return index_path.string();
}

static Table read_table(io::IO &index_io)
{
    auto data = index_io.read(index_io.size() - 4);
    auto seed = index_io.read_u32_le();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(data.size()))
        data[i] ^= key[lcg.next() % key.size()];

    Table table;
    io::BufferedIO data_io(data);
    size_t min_offset = 0xFFFFFFFF;
    while (true)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::sjis_to_utf8(data_io.read_to_zero(20)).str();
        if (!entry->name.size())
            break;
        entry->offset = data_io.read_u32_le();
        entry->size = data_io.read_u32_le();
        min_offset = std::min(min_offset, entry->offset);
        table.push_back(std::move(entry));
    }
    for (auto &entry : table)
        entry->offset -= min_offset;
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

bool PakArchive::is_recognized_internal(File &arc_file) const
{
    return boost::filesystem::exists(get_path_to_index(arc_file.name))
        && arc_file.has_extension("pak");
}

void PakArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    io::FileIO index_io(get_path_to_index(arc_file.name), io::FileMode::Read);
    auto table = read_table(index_io);

    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<PakArchive>("eagls/pak");
