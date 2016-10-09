#include "dec/valkyria/odn_archive_decoder.h"
#include <map>
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::valkyria;

namespace
{
    enum class OdnVariant : u8
    {
        Variant1,
        Variant2,
        Variant3,
    };
}

static size_t hex_to_int(const bstr &input)
{
    size_t ret = 0;
    for (const auto c : input)
    {
        ret *= 16;
        if (c >= 'a' && c <= 'f') ret += c + 10 - 'a';
        if (c >= 'A' && c <= 'F') ret += c + 10 - 'A';
        if (c >= '0' && c <= '9') ret += c - '0';
    }
    return ret;
}

bool OdnArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("odn");
}

static OdnVariant guess_odn_variant(io::BaseByteStream &input_stream)
{
    if (input_stream.seek(8).read(8) == "00000000"_b)
        return OdnVariant::Variant1;

    const auto likely_file1_prefix = input_stream.seek(0).read(4);
    const auto maybe_file2_prefix1 = input_stream.seek(16).read(4);
    const auto maybe_file2_prefix2 = input_stream.seek(24).read(4);

    if (likely_file1_prefix == maybe_file2_prefix1)
        return OdnVariant::Variant2;

    if (likely_file1_prefix == maybe_file2_prefix2)
        return OdnVariant::Variant3;

    throw err::RecognitionError("Not an ODN archive");
}

static void fill_sizes(
    const io::BaseByteStream &input_stream,
    std::vector<std::unique_ptr<dec::ArchiveEntry>> &entries)
{
    if (!entries.size())
        return;
    for (const auto i : algo::range(1, entries.size()))
    {
        auto prev_entry
            = static_cast<dec::PlainArchiveEntry*>(entries[i - 1].get());
        auto current_entry
            = static_cast<dec::PlainArchiveEntry*>(entries[i].get());
        prev_entry->size = current_entry->offset - prev_entry->offset;
    }
    auto last_entry = static_cast<dec::PlainArchiveEntry*>(
        entries.back().get());
    last_entry->size = input_stream.size() - last_entry->offset;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v1(
    io::BaseByteStream &input_stream)
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    while (true)
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = input_stream.read(8).str();
        entry->offset = hex_to_int(input_stream.read(8));
        if (entry->path.str().substr(0, 4) == "END_")
            break;
        meta->entries.push_back(std::move(entry));
    }
    for (auto &e : meta->entries)
    {
        auto entry = static_cast<dec::PlainArchiveEntry*>(e.get());
        entry->offset += input_stream.pos();
    }
    fill_sizes(input_stream, meta->entries);
    return meta;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v2(
    io::BaseByteStream &input_stream)
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    const auto data_pos = hex_to_int(input_stream.seek(8).read(8));
    input_stream.seek(0);
    while (input_stream.pos() < data_pos)
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = input_stream.read(8).str();
        entry->offset = hex_to_int(input_stream.read(8));
        meta->entries.push_back(std::move(entry));
    }
    fill_sizes(input_stream, meta->entries);
    return meta;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v3(
    io::BaseByteStream &input_stream)
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    const auto data_pos = hex_to_int(input_stream.seek(8).read(8));
    input_stream.seek(0);
    while (input_stream.pos() < data_pos)
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = input_stream.read(8).str();
        entry->offset = hex_to_int(input_stream.read(8));
        input_stream.skip(8);
        meta->entries.push_back(std::move(entry));
    }
    fill_sizes(input_stream, meta->entries);
    return meta;
}

std::unique_ptr<dec::ArchiveMeta> OdnArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto variant = guess_odn_variant(input_file.stream);
    switch (variant)
    {
        case OdnVariant::Variant1:
            return read_meta_v1(input_file.stream);
        case OdnVariant::Variant2:
            return read_meta_v2(input_file.stream);
        case OdnVariant::Variant3:
            return read_meta_v3(input_file.stream);
        default:
            throw std::logic_error("Invalid archive type");
    }
}

std::unique_ptr<io::File> OdnArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto ret = std::make_unique<io::File>(entry->path, data);
    ret->guess_extension();
    return ret;
}

static auto _ = dec::register_decoder<OdnArchiveDecoder>("valkyria/odn");
