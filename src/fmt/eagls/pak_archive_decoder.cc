#include "fmt/eagls/pak_archive_decoder.h"
#include <algorithm>
#include "io/file_stream.h"
#include "io/filesystem.h"
#include "io/memory_stream.h"
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

static io::path get_path_to_index(const io::path &path_to_data)
{
    io::path index_path(path_to_data);
    index_path.change_extension("idx");
    return index_path;
}

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return io::exists(get_path_to_index(input_file.name))
        && input_file.name.has_extension("pak");
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    io::FileStream index_stream(
        get_path_to_index(input_file.name), io::FileMode::Read);

    auto data = index_stream.read(index_stream.size() - 4);
    auto seed = index_stream.read_u32_le();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(data.size()))
        data[i] ^= key[lcg.next() % key.size()];

    io::MemoryStream data_stream(data);
    size_t min_offset = 0xFFFFFFFF;
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(data_stream.read_to_zero(20)).str();
        if (!entry->name.size())
            break;
        entry->offset = data_stream.read_u32_le();
        entry->size = data_stream.read_u32_le();
        min_offset = std::min(min_offset, entry->offset);
        meta->entries.push_back(std::move(entry));
    }

    // According to Crass min_offset is calculated differently for some games.
    for (auto &entry : meta->entries)
        static_cast<ArchiveEntryImpl*>(entry.get())->offset -= min_offset;

    return meta;
}

std::unique_ptr<io::File> PakArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"eagls/gr", "eagls/pak-script"};
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("eagls/pak");
