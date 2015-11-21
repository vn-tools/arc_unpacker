#include "fmt/team_shanghai_alice/anm_archive_decoder.h"
#include <algorithm>
#include <map>
#include "err.h"
#include "fmt/naming_strategies.h"
#include "util/file_from_grid.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::team_shanghai_alice;

namespace
{
    struct TextureInfo final
    {
        std::string name;
        size_t width, height;
        size_t x, y;
        size_t format;
        int version;
        size_t texture_offset;
        bool has_data;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        std::vector<TextureInfo> texture_info_list;
    };
}

static const bstr texture_magic = "THTX"_b;

static std::string read_name(io::Stream &input, size_t offset)
{
    std::string name;
    input.peek(offset, [&]() { name = input.read_to_zero().str(); });
    return name;
}

static size_t read_old_texture_info(
    TextureInfo &texture_info, io::Stream &input, size_t base_offset)
{
    input.skip(4); // sprite count
    input.skip(4); // script count
    input.skip(4); // zero

    texture_info.width = input.read_u32_le();
    texture_info.height = input.read_u32_le();
    texture_info.format = input.read_u32_le();
    texture_info.x = input.read_u16_le();
    texture_info.y = input.read_u16_le();
    // input.skip(4);

    size_t name_offset1 = base_offset + input.read_u32_le();
    input.skip(4);
    size_t name_offset2 = base_offset + input.read_u32_le();
    texture_info.name = read_name(input, name_offset1);

    texture_info.version = input.read_u32_le();
    input.skip(4);
    texture_info.texture_offset = base_offset + input.read_u32_le();
    texture_info.has_data = input.read_u32_le() > 0;

    return base_offset + input.read_u32_le();
}

static size_t read_new_texture_info(
    TextureInfo &texture_info, io::Stream &input, size_t base_offset)
{
    texture_info.version = input.read_u32_le();
    input.skip(2); // sprite count
    input.skip(2); // script count
    input.skip(2); // zero

    texture_info.width = input.read_u16_le();
    texture_info.height = input.read_u16_le();
    texture_info.format = input.read_u16_le();
    size_t name_offset = base_offset + input.read_u32_le();
    texture_info.name = read_name(input, name_offset);
    texture_info.x = input.read_u16_le();
    texture_info.y = input.read_u16_le();
    input.skip(4);

    texture_info.texture_offset = base_offset + input.read_u32_le();
    texture_info.has_data = input.read_u16_le() > 0;
    input.skip(2);

    return base_offset + input.read_u32_le();
}

static void write_pixels(
    io::Stream &input,
    const TextureInfo &texture_info,
    pix::Grid &pixels,
    size_t stride)
{
    if (!texture_info.has_data)
        return;

    input.seek(texture_info.texture_offset);
    if (input.read(texture_magic.size()) != texture_magic)
        throw err::CorruptDataError("Corrupt texture data");
    input.skip(2);
    auto format = input.read_u16_le();
    auto width = input.read_u16_le();
    auto height = input.read_u16_le();
    auto data_size = input.read_u32_le();
    auto data = input.read(data_size);
    auto data_ptr = data.get<const u8>();

    for (auto y : util::range(height))
    for (auto x : util::range(width))
    {
        pix::Pixel color;
        switch (format)
        {
            case 1:
                color = pix::read<pix::Format::BGRA8888>(data_ptr);
                break;

            case 3:
                color = pix::read<pix::Format::BGR565>(data_ptr);
                break;

            case 5:
                color = pix::read<pix::Format::BGRA4444>(data_ptr);
                break;

            case 7:
                color = pix::read<pix::Format::Gray8>(data_ptr);
                break;

            default:
                throw err::NotSupportedError(util::format(
                    "Unknown color format: %d", format));
        }

        pixels.at(x + texture_info.x, y + texture_info.y) = color;
    }
}

static std::vector<TextureInfo> read_texture_info_list(io::Stream &input)
{
    std::vector<TextureInfo> texture_info_list;
    u32 base_offset = 0;
    while (true)
    {
        TextureInfo texture_info;

        input.seek(base_offset);
        input.skip(8);
        bool use_old = input.read_u32_le() == 0;

        input.seek(base_offset);
        size_t next_offset = use_old
            ? read_old_texture_info(texture_info, input, base_offset)
            : read_new_texture_info(texture_info, input, base_offset);

        if (texture_info.has_data)
            if (texture_info.width > 0 && texture_info.height > 0)
                texture_info_list.push_back(texture_info);

        if (next_offset == base_offset)
            break;
        base_offset = next_offset;
    }
    return texture_info_list;
}

std::unique_ptr<INamingStrategy> AnmArchiveDecoder::naming_strategy() const
{
    return std::make_unique<RootNamingStrategy>();
}

bool AnmArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("anm");
}

std::unique_ptr<fmt::ArchiveMeta>
    AnmArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto texture_info_list = read_texture_info_list(input_file.stream);

    std::map<std::string, std::vector<TextureInfo>> map;
    for (auto &texture_info : texture_info_list)
        map[texture_info.name].push_back(texture_info);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto &kv : map)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = kv.first;
        entry->texture_info_list = kv.second;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> AnmArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    size_t width = 0;
    size_t height = 0;
    for (auto &texture_info : entry->texture_info_list)
    {
        input_file.stream.peek(texture_info.texture_offset, [&]()
        {
            input_file.stream.skip(texture_magic.size());
            input_file.stream.skip(2);
            input_file.stream.skip(2);
            size_t chunk_width = input_file.stream.read_u16_le();
            size_t chunk_height = input_file.stream.read_u16_le();
            width = std::max(width, texture_info.x + chunk_width);
            height = std::max(height, texture_info.y + chunk_height);
        });
    }

    pix::Grid pixels(width, height);
    for (auto &texture_info : entry->texture_info_list)
        write_pixels(input_file.stream, texture_info, pixels, width);

    return util::file_from_grid(pixels, entry->name);
}

static auto dummy
    = fmt::register_fmt<AnmArchiveDecoder>("team-shanghai-alice/anm");
