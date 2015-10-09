#include "fmt/tactics/arc_archive_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::tactics;

static const bstr magic = "TACTICS_ARC_FILE"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t size_comp;
        size_t size_orig;
        size_t offset;
        bstr key;
    };
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta_v0(File &arc_file)
{
    auto size_comp = arc_file.io.read_u32_le();
    auto size_orig = arc_file.io.read_u32_le();
    auto file_count = arc_file.io.read_u32_le();
    if (size_comp > 1024 * 1024 * 10)
        throw err::BadDataSizeError();

    arc_file.io.skip(4);
    auto table_buf = arc_file.io.read(size_comp);
    auto data_start = arc_file.io.tell();

    for (auto &c : table_buf)
        c ^= 0xFF;
    io::BufferedIO table_io(
        util::pack::lzss_decompress_bytewise(table_buf, size_orig));

    auto key = table_io.read_to_zero();
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = table_io.read_u32_le() + data_start;
        entry->size_comp = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        entry->key = key;
        auto name_size = table_io.read_u32_le();

        table_io.skip(8);
        entry->name = util::sjis_to_utf8(table_io.read(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta_v1(File &arc_file)
{
    static const bstr key = "mlnebzqm"_b; // found in .exe
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    while (!arc_file.io.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->size_comp = arc_file.io.read_u32_le();
        if (!entry->size_comp)
            break;
        entry->size_orig = arc_file.io.read_u32_le();
        auto name_size = arc_file.io.read_u32_le();
        arc_file.io.skip(8);
        entry->name = util::sjis_to_utf8(arc_file.io.read(name_size)).str();
        entry->offset = arc_file.io.tell();
        entry->key = key;
        arc_file.io.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

bool ArcArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    ArcArchiveDecoder::read_meta_impl(File &arc_file) const
{
    std::vector<std::function<std::unique_ptr<fmt::ArchiveMeta>(File &)>>
        meta_readers
        {
            read_meta_v0,
            read_meta_v1
        };

    for (auto meta_reader : meta_readers)
    {
        arc_file.io.seek(magic.size());
        try
        {
            return meta_reader(arc_file);
        }
        catch (std::exception &e)
        {
            continue;
        }
    }

    throw err::NotSupportedError("Archive is encrypted in unknown way.");
}

std::unique_ptr<File> ArcArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);
    if (entry->key.size())
        for (auto i : util::range(data.size()))
            data[i] ^= entry->key[i % entry->key.size()];
    if (entry->size_orig)
        data = util::pack::lzss_decompress_bytewise(data, entry->size_orig);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return { "ms/dds" };
}

static auto dummy = fmt::register_fmt<ArcArchiveDecoder>("tactics/arc");
