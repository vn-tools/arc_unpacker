#include "dec/kaguya/plt_image_archive_decoder.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "PL00"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
        size_t width;
        size_t height;
        size_t channels;
        std::unique_ptr<res::Image> mask;
    };
}

algo::NamingStrategy PltImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

bool PltImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PltImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();
    const auto x = input_file.stream.read_le<u32>();
    const auto y = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        input_file.stream.skip(8);
        entry->width = input_file.stream.read_le<u32>();
        entry->height = input_file.stream.read_le<u32>();
        entry->channels = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.tell();
        entry->size = entry->channels * entry->width * entry->height;
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PltImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    res::PixelFormat fmt;
    if (entry->channels == 3)
        fmt = res::PixelFormat::BGR888;
    else if (entry->channels == 4)
        fmt = res::PixelFormat::BGRA8888;
    else
        throw err::UnsupportedChannelCountError(entry->channels);
    res::Image image(
        entry->width,
        entry->height,
        input_file.stream.seek(entry->offset).read(entry->size),
        fmt);
    image.flip_vertically();
    return enc::png::PngImageEncoder().encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<PltImageArchiveDecoder>("kaguya/plt");
