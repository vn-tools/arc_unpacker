#include "fmt/ast/arc_archive_decoder.h"
#include <algorithm>
#include "util/pack/lzss.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::ast;

static const bstr magic1 = "ARC1"_b;
static const bstr magic2 = "ARC2"_b;

namespace
{
    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        int version;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_orig;
        size_t size_comp;
    };
}

static void xor_data(bstr &data)
{
    for (auto &c : data)
        c ^= 0xFF;
}

bool ArcArchiveDecoder::is_recognized_impl(File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic1.size()) == magic1)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic2.size()) == magic2;
}

std::unique_ptr<fmt::ArchiveMeta>
    ArcArchiveDecoder::read_meta_impl(File &input_file) const
{
    auto meta = std::make_unique<ArchiveMetaImpl>();

    input_file.stream.seek(3);
    meta->version = input_file.stream.read_u8() - '0';

    ArchiveEntryImpl *last_entry = nullptr;
    const auto file_count = input_file.stream.read_u32_le();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = input_file.stream.read_u32_le();
        entry->size_orig = input_file.stream.read_u32_le();
        auto name = input_file.stream.read(input_file.stream.read_u8());
        if (meta->version == 2)
            xor_data(name);
        entry->name = util::sjis_to_utf8(name).str();
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
        last_entry->size_comp = input_file.stream.size() - last_entry->offset;

    return std::move(meta);
}

std::unique_ptr<File> ArcArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);

    auto output_file = std::make_unique<File>();
    output_file->name = entry->name;

    if (meta->version == 2)
    {
        if (entry->size_comp != entry->size_orig)
            data = util::pack::lzss_decompress_bytewise(data, entry->size_orig);

        const auto header = data.substr(0, 4);
        bool known = false;
        known |= header == "\x89PNG"_b;
        known |= header == "RIFF"_b;
        known |= header == "OggS"_b;
        if (!known)
            xor_data(data);
    }

    output_file->stream.write(data);
    return output_file;
}

static auto dummy = fmt::register_fmt<ArcArchiveDecoder>("ast/arc");
