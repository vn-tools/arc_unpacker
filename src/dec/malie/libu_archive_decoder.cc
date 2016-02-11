#include "dec/malie/libu_archive_decoder.h"
#include <cstring>
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

    // Rather than decrypting to bstr, the decryption is implemented as stream,
    // so that huge files occupy as little memory as possible
    class CamelliaStream final : public io::BaseByteStream
    {
    public:
        CamelliaStream(
            io::BaseByteStream &parent_stream, const LibPlugin &key);
        CamelliaStream(
            io::BaseByteStream &parent_stream,
            const LibPlugin &key,
            const size_t offset,
            const size_t size);
        ~CamelliaStream();

        size_t size() const override;
        size_t tell() const override;
        std::unique_ptr<BaseByteStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;
        void seek_impl(const size_t offset) override;
        void resize_impl(const size_t new_size) override;

    private:
        const LibPlugin key;
        std::unique_ptr<algo::crypt::Camellia> camellia;
        std::unique_ptr<io::BaseByteStream> parent_stream;
        const size_t parent_stream_offset;
        const size_t parent_stream_size;
    };
}

CamelliaStream::CamelliaStream(
    io::BaseByteStream &parent_stream, const LibPlugin &key)
        : CamelliaStream(parent_stream, key, 0, parent_stream.size())
{
}

CamelliaStream::CamelliaStream(
    io::BaseByteStream &parent_stream,
    const LibPlugin &key,
    const size_t offset,
    const size_t size) :
        key(key),
        parent_stream(parent_stream.clone()),
        parent_stream_offset(offset),
        parent_stream_size(size)
{
    if (key.size())
        camellia = std::make_unique<algo::crypt::Camellia>(key);
}

CamelliaStream::~CamelliaStream()
{
}

void CamelliaStream::seek_impl(const size_t offset)
{
    parent_stream->seek(parent_stream_offset + offset);
}

void CamelliaStream::read_impl(void *destination, const size_t size)
{
    if (!camellia)
    {
        const auto chunk = parent_stream->read(size);
        std::memcpy(destination, chunk.get<u8>(), size);
        return;
    }

    const auto old_pos = parent_stream->tell();
    const auto offset_pad  = parent_stream->tell() & 0xF;
    const auto offset_start = parent_stream->tell() & ~0xF;
    const auto aligned_size = (offset_pad + size + 0xF) & ~0xF;
    const auto block_count = (aligned_size + 0xF) / 0x10;
    if (block_count == 0)
        return;

    parent_stream->seek(parent_stream_offset
        + ((parent_stream->tell() - parent_stream_offset) - offset_pad));

    io::MemoryStream output_stream;
    output_stream.resize(block_count * 16);
    output_stream.seek(0);
    for (const auto i : algo::range(block_count))
    {
        u32 input_block[4];
        u32 output_block[4];
        for (const auto j : algo::range(4))
            input_block[j] = parent_stream->read_le<u32>();
        camellia->decrypt_block_128(
            offset_start + i * 0x10, input_block, output_block);
        for (const auto j : algo::range(4))
            output_stream.write_be<u32>(output_block[j]);
    }
    const auto chunk = output_stream.seek(offset_pad).read(size);
    std::memcpy(destination, chunk.get<u8>(), size);
    parent_stream->seek(old_pos + size);
}

void CamelliaStream::write_impl(const void *source, const size_t size)
{
    throw err::NotSupportedError("Not implemented");
}

size_t CamelliaStream::tell() const
{
    return parent_stream->tell() - parent_stream_offset;
}

size_t CamelliaStream::size() const
{
    return parent_stream_size;
}

void CamelliaStream::resize_impl(const size_t new_size)
{
    parent_stream->resize(new_size);
}

std::unique_ptr<io::BaseByteStream> CamelliaStream::clone() const
{
    return std::make_unique<CamelliaStream>(
        *parent_stream, key, parent_stream_offset, parent_stream_size);
}

bool LibuArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    for (const auto &plugin : plugin_manager.get_all())
    {
        CamelliaStream camellia_stream(input_file.stream, plugin);
        const auto maybe_magic = camellia_stream.seek(0).read(4);
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

    CamelliaStream camellia_stream(input_file.stream, meta->plugin);
    camellia_stream.seek(magic.size());
    const auto version = camellia_stream.read_le<u32>();
    const auto file_count = camellia_stream.read_le<u32>();
    camellia_stream.skip(4);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
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
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    return std::make_unique<io::File>(
        entry->path,
        std::make_unique<CamelliaStream>(
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
