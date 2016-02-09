#include "dec/malie/libu_archive_decoder.h"
#include "algo/crypt/camellia.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::malie;

static const auto magic = "LIBU"_b;

namespace
{
    struct ArchiveMetaImpl final : dec::ArchiveMeta
    {
        LibPlugin plugin;
    };

    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(
    const LibPlugin &key,
    io::BaseByteStream &input_stream,
    io::BaseByteStream &output_stream,
    const size_t size)
{
    if (key.empty())
    {
        output_stream.write(input_stream.read(size));
        return;
    }

    algo::crypt::Camellia camellia(key);

    const auto offset_pad  = input_stream.tell() & 0xF;
    const auto offset_start = input_stream.tell() & ~0xF;
    const auto aligned_size = (offset_pad + size + 0xF) & ~0xF;
    const auto block_count = (aligned_size + 0xF) / 0x10;
    if (block_count == 0)
        return;

    input_stream.seek(input_stream.tell() - offset_pad);
    for (const auto i : algo::range(block_count))
    {
        u32 input_block[4];
        u32 output_block[4];
        for (const auto j : algo::range(4))
            input_block[j] = input_stream.read_le<u32>();
        camellia.decrypt_block_128(
            offset_start + i * 0x10,
            input_block,
            output_block);
        for (const auto j : algo::range(4))
            output_stream.write_be<u32>(output_block[j]);

        if (i == 0)
        {
            const auto first_block = output_stream.seek(0).read_to_eof();
            const auto block_part = first_block.substr(offset_pad);
            output_stream.resize(0);
            output_stream.write(block_part);
            output_stream.resize(size);
        }
    }
}

static bstr decrypt(
    const LibPlugin &key,
    io::BaseByteStream &input_stream,
    const size_t size)
{
    io::MemoryStream output_stream;
    decrypt(key, input_stream, output_stream, size);
    return output_stream.seek(0).read(size);
}

bool LibuArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    for (const auto &plugin : plugin_manager.get_all())
    {
        input_file.stream.seek(0);
        const auto maybe_magic = decrypt(plugin, input_file.stream, 4);
        if (maybe_magic == magic)
            return true;
    }
    return false;
}

std::unique_ptr<dec::ArchiveMeta> LibuArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMetaImpl>();

    const auto maybe_magic = input_file.stream.seek(0).read(magic.size());
    meta->plugin = maybe_magic == magic
        ? plugin_manager.get("noop")
        : plugin_manager.get();
    input_file.stream.seek(0);

    io::MemoryStream header_stream(
        decrypt(meta->plugin, input_file.stream, 0x10));
    header_stream.seek(magic.size());
    const auto version = header_stream.read_le<u32>();
    const auto file_count = header_stream.read_le<u32>();
    const auto table_size = file_count * 80;

    io::MemoryStream table_stream(
        decrypt(meta->plugin, input_file.stream, table_size));

    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = algo::utf16_to_utf8(table_stream.read(68)).str(true);
        entry->size = table_stream.read_le<u32>();
        entry->offset = table_stream.read_le<u32>();
        table_stream.skip(4);
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
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto output_file = std::make_unique<io::File>(entry->path, ""_b);
    decrypt(meta->plugin, input_file.stream, output_file->stream, entry->size);
    return output_file;
}

std::vector<std::string> LibuArchiveDecoder::get_linked_formats() const
{
    return {"malie/libu", "malie/mgf"};
}

static auto _ = dec::register_decoder<LibuArchiveDecoder>("malie/libu");
