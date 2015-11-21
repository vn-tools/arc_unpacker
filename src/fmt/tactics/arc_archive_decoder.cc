#include "fmt/tactics/arc_archive_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
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

static std::unique_ptr<fmt::ArchiveMeta> read_meta_v0(File &input_file)
{
    auto size_comp = input_file.stream.read_u32_le();
    auto size_orig = input_file.stream.read_u32_le();
    auto file_count = input_file.stream.read_u32_le();
    if (size_comp > 1024 * 1024 * 10)
        throw err::BadDataSizeError();

    input_file.stream.skip(4);
    auto table_buf = input_file.stream.read(size_comp);
    auto data_start = input_file.stream.tell();

    for (auto &c : table_buf)
        c ^= 0xFF;
    io::MemoryStream table_stream(
        util::pack::lzss_decompress_bytewise(table_buf, size_orig));

    auto key = table_stream.read_to_zero();
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = table_stream.read_u32_le() + data_start;
        entry->size_comp = table_stream.read_u32_le();
        entry->size_orig = table_stream.read_u32_le();
        entry->key = key;
        auto name_size = table_stream.read_u32_le();

        table_stream.skip(8);
        entry->name = util::sjis_to_utf8(table_stream.read(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta_v1(File &input_file)
{
    static const bstr key = "mlnebzqm"_b; // found in .exe
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->size_comp = input_file.stream.read_u32_le();
        if (!entry->size_comp)
            break;
        entry->size_orig = input_file.stream.read_u32_le();
        auto name_size = input_file.stream.read_u32_le();
        input_file.stream.skip(8);
        entry->name = util::sjis_to_utf8(
            input_file.stream.read(name_size)).str();
        entry->offset = input_file.stream.tell();
        entry->key = key;
        input_file.stream.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

bool ArcArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    ArcArchiveDecoder::read_meta_impl(File &input_file) const
{
    std::vector<std::function<std::unique_ptr<fmt::ArchiveMeta>(File &)>>
        meta_readers
        {
            read_meta_v0,
            read_meta_v1
        };

    for (auto meta_reader : meta_readers)
    {
        input_file.stream.seek(magic.size());
        try
        {
            return meta_reader(input_file);
        }
        catch (std::exception &e)
        {
            continue;
        }
    }

    throw err::NotSupportedError("Archive is encrypted in unknown way.");
}

std::unique_ptr<File> ArcArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);
    if (entry->key.size())
        for (auto i : util::range(data.size()))
            data[i] ^= entry->key[i % entry->key.size()];
    if (entry->size_orig)
        data = util::pack::lzss_decompress_bytewise(data, entry->size_orig);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"microsoft/dds"};
}

static auto dummy = fmt::register_fmt<ArcArchiveDecoder>("tactics/arc");
