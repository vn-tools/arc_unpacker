#include "dec/nitroplus/npa_sg_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::nitroplus;

static const bstr key = "\xBD\xAA\xBC\xB4\xAB\xB6\xBC\xB4"_b;

static void decrypt(bstr &data)
{
    for (const auto i : algo::range(data.size()))
        data[i] ^= key[i % key.size()];
}

bool NpaSgArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("npa"))
        return false;
    const auto table_size = input_file.stream.read_le<u32>();
    return table_size < input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> NpaSgArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto table_size = input_file.stream.read_le<u32>();
    auto table_data = input_file.stream.read(table_size);
    decrypt(table_data);
    io::MemoryByteStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = table_stream.read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        const auto name_size = table_stream.read_le<u32>();
        entry->path = algo::utf16_to_utf8(table_stream.read(name_size)).str();
        entry->size = table_stream.read_le<u32>();
        entry->offset = table_stream.read_le<u32>();
        table_stream.skip(4);
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> NpaSgArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    decrypt(data);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<NpaSgArchiveDecoder>("nitroplus/npa-sg");
