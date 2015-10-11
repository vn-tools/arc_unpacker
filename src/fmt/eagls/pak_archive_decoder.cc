#include "fmt/eagls/pak_archive_decoder.h"
#include <algorithm>
#include <boost/filesystem.hpp>
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
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static std::string get_path_to_index(const std::string &path_to_data)
{
    boost::filesystem::path index_path(path_to_data);
    index_path.replace_extension("idx");
    return index_path.string();
}

bool PakArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return boost::filesystem::exists(get_path_to_index(arc_file.name))
        && arc_file.has_extension("pak");
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(File &arc_file) const
{
    io::FileIO index_io(get_path_to_index(arc_file.name), io::FileMode::Read);

    auto data = index_io.read(index_io.size() - 4);
    auto seed = index_io.read_u32_le();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(data.size()))
        data[i] ^= key[lcg.next() % key.size()];

    io::BufferedIO data_io(data);
    size_t min_offset = 0xFFFFFFFF;
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(data_io.read_to_zero(20)).str();
        if (!entry->name.size())
            break;
        entry->offset = data_io.read_u32_le();
        entry->size = data_io.read_u32_le();
        min_offset = std::min(min_offset, entry->offset);
        meta->entries.push_back(std::move(entry));
    }

    // According to Crass min_offset is calculated differently for some games.
    for (auto &entry : meta->entries)
        static_cast<ArchiveEntryImpl*>(entry.get())->offset -= min_offset;

    return meta;
}

std::unique_ptr<File> PakArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return { "eagls/gr", "eagls/pak-script" };
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("eagls/pak");
