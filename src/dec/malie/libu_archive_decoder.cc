#include "dec/malie/libu_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/malie/common/camellia_stream.h"

using namespace au;
using namespace au::dec::malie;

static const auto magic = "LIBU"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::vector<u32> plugin;
    };
}

bool LibuArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    for (const auto &plugin : plugin_manager.get_all())
    {
        common::CamelliaStream camellia_stream(input_file.stream, plugin);
        const auto maybe_magic = camellia_stream.seek(0).read(4);
        if (maybe_magic == magic)
            return true;
    }
    return false;
}

std::unique_ptr<dec::ArchiveMeta> LibuArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();

    const auto maybe_magic = input_file.stream.seek(0).read(magic.size());
    meta->plugin = maybe_magic == magic
        ? plugin_manager.get("noop")
        : plugin_manager.get();

    common::CamelliaStream camellia_stream(input_file.stream, meta->plugin);
    camellia_stream.seek(magic.size());
    const auto version = camellia_stream.read_le<u32>();
    const auto file_count = camellia_stream.read_le<u32>();
    camellia_stream.skip(4);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::utf16_to_utf8(camellia_stream.read(68)).str(true);
        entry->size = camellia_stream.read_le<u32>();
        entry->offset = camellia_stream.read_le<u32>();
        camellia_stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> LibuArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    return std::make_unique<io::File>(
        entry->path,
        std::make_unique<common::CamelliaStream>(
            input_file.stream,
            meta->plugin,
            entry->offset,
            entry->size));
}

std::vector<std::string> LibuArchiveDecoder::get_linked_formats() const
{
    return {"malie/libu", "malie/mgf"};
}

static auto _ = dec::register_decoder<LibuArchiveDecoder>("malie/libu");
