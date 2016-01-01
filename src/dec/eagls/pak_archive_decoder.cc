#include "dec/eagls/pak_archive_decoder.h"
#include "algo/crypt/lcg.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/file_stream.h"
#include "io/file_system.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::eagls;

static const bstr key = "1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik,9ol.0p;/-@:^[]"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
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
    return io::exists(get_path_to_index(input_file.path))
        && input_file.path.has_extension("pak");
}

std::unique_ptr<dec::ArchiveMeta> PakArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    io::FileStream index_stream(
        get_path_to_index(input_file.path), io::FileMode::Read);

    auto data = index_stream.read(index_stream.size() - 4);
    auto seed = index_stream.read_u32_le();
    algo::crypt::Lcg lcg(algo::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : algo::range(data.size()))
        data[i] ^= key[lcg.next() % key.size()];

    io::MemoryStream data_stream(data);
    size_t min_offset = 0xFFFFFFFF;
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = algo::sjis_to_utf8(data_stream.read_to_zero(20)).str();
        if (entry->path.str().empty())
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
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"eagls/gr", "eagls/pak-script"};
}

static auto _ = dec::register_decoder<PakArchiveDecoder>("eagls/pak");
