#include "fmt/leaf/pak2_group/pak2_texture_archive_decoder.h"
#include "err.h"
#include "fmt/naming_strategies.h"
#include "util/file_from_grid.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "\x88\x33\x67\x82"_b;
static const bstr canvas_magic1 = "\x70\x2B\xCD\xC8"_b;
static const bstr canvas_magic2 = "\x03\xC5\x0D\xA6"_b;

namespace
{
    struct Chunk final
    {
        size_t offset, size;
        int x, y;
        size_t width, height;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        std::vector<Chunk> chunks;
    };
}

bool Pak2TextureArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak2TextureArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();

    input_file.stream.seek(24);
    const auto image_count = input_file.stream.read_u16_le();
    const auto chunk_count = input_file.stream.read_u16_le();
    input_file.stream.skip(4);
    auto last_chunk_offset = input_file.stream.read_u16_le();

    const auto data_offset = input_file.stream.tell()
        + image_count * 2
        + chunk_count * 36
        + 36;

    std::vector<size_t> image_chunk_counts;
    for (const auto i : util::range(image_count))
    {
        const auto chunk_offset = input_file.stream.read_u16_le();
        image_chunk_counts.push_back(chunk_offset - last_chunk_offset);
        last_chunk_offset = chunk_offset;
    }

    int i =0;
    for (const auto image_chunk_count : image_chunk_counts)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        for (const auto j : util::range(image_chunk_count))
        {
            Chunk chunk;
            input_file.stream.skip(8);
            chunk.x = static_cast<s16>(input_file.stream.read_u16_le());
            chunk.y = static_cast<s16>(input_file.stream.read_u16_le());
            input_file.stream.skip(4);
            chunk.width = input_file.stream.read_u16_le();
            chunk.height = input_file.stream.read_u16_le();
            input_file.stream.skip(4);
            chunk.offset = input_file.stream.read_u32_le() + data_offset;
            input_file.stream.skip(8);
            entry->chunks.push_back(chunk);
        }
        if (!entry->chunks.empty())
            meta->entries.push_back(std::move(entry));
    }

    const auto base_name = io::path(input_file.name).stem();
    for (auto i : util::range(meta->entries.size()))
        meta->entries[i]->name = meta->entries.size() > 1
            ? util::format("%s_%03d", base_name.c_str(), i)
            : base_name;

    return meta;
}

std::unique_ptr<io::File> Pak2TextureArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    int min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    for (const auto &chunk : entry->chunks)
    {
        min_x = std::min<int>(min_x, chunk.x);
        min_y = std::min<int>(min_y, chunk.y);
        max_x = std::max<int>(max_x, chunk.x + chunk.width);
        max_y = std::max<int>(max_y, chunk.y + chunk.height);
    }
    const auto width = max_x - min_x;
    const auto height = max_y - min_y;
    pix::Grid image(width, height);
    for (auto &c : image)
    {
        c.r = c.g = c.b = 0;
        c.a = 0xFF;
    }
    for (const auto &chunk : entry->chunks)
    {
        input_file.stream.seek(chunk.offset);
        auto chunk_image = pix::Grid(
            chunk.width,
            chunk.height,
            input_file.stream.read(chunk.width * chunk.height * 2),
            pix::Format::BGRnA5551);
        chunk_image.flip_vertically();
        image.paste(chunk_image, chunk.x, chunk.y);
    }
    return util::file_from_grid(image, entry->name);
}

std::unique_ptr<fmt::INamingStrategy>
    Pak2TextureArchiveDecoder::naming_strategy() const
{
    return std::make_unique<SiblingNamingStrategy>();
}

static auto dummy
    = fmt::register_fmt<Pak2TextureArchiveDecoder>("leaf/pak2-texture");
