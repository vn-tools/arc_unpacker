// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/team_shanghai_alice/anm_archive_decoder.h"
#include <map>
#include "algo/format.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec;
using namespace au::dec::team_shanghai_alice;

namespace
{
    struct TextureInfo final
    {
        std::string name;
        size_t width, height;
        size_t x, y;
        size_t format;
        int version;
        uoff_t texture_offset;
        bool has_data;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        std::vector<TextureInfo> texture_info_list;
    };
}

static const bstr texture_magic = "THTX"_b;

static std::string read_name(
    io::BaseByteStream &input_stream, const uoff_t offset)
{
    std::string name;
    input_stream.peek(
        offset, [&]() { name = input_stream.read_to_zero().str(); });
    return name;
}

static size_t read_old_texture_info(
    TextureInfo &texture_info,
    io::BaseByteStream &input_stream,
    const uoff_t base_offset)
{
    input_stream.skip(4); // sprite count
    input_stream.skip(4); // script count
    input_stream.skip(4); // zero

    texture_info.width = input_stream.read_le<u32>();
    texture_info.height = input_stream.read_le<u32>();
    texture_info.format = input_stream.read_le<u32>();
    texture_info.x = input_stream.read_le<u16>();
    texture_info.y = input_stream.read_le<u16>();
    // input_stream.skip(4);

    const auto name_offset1 = base_offset + input_stream.read_le<u32>();
    input_stream.skip(4);
    const auto name_offset2 = base_offset + input_stream.read_le<u32>();
    texture_info.name = read_name(input_stream, name_offset1);

    texture_info.version = input_stream.read_le<u32>();
    input_stream.skip(4);
    texture_info.texture_offset = base_offset + input_stream.read_le<u32>();
    texture_info.has_data = input_stream.read_le<u32>() > 0;

    return base_offset + input_stream.read_le<u32>();
}

static size_t read_new_texture_info(
    TextureInfo &texture_info,
    io::BaseByteStream &input_stream,
    const uoff_t base_offset)
{
    texture_info.version = input_stream.read_le<u32>();
    input_stream.skip(2); // sprite count
    input_stream.skip(2); // script count
    input_stream.skip(2); // zero

    texture_info.width = input_stream.read_le<u16>();
    texture_info.height = input_stream.read_le<u16>();
    texture_info.format = input_stream.read_le<u16>();
    const auto name_offset = base_offset + input_stream.read_le<u32>();
    texture_info.name = read_name(input_stream, name_offset);
    texture_info.x = input_stream.read_le<u16>();
    texture_info.y = input_stream.read_le<u16>();
    input_stream.skip(4);

    texture_info.texture_offset = base_offset + input_stream.read_le<u32>();
    texture_info.has_data = input_stream.read_le<u16>() > 0;
    input_stream.skip(2);

    return base_offset + input_stream.read_le<u32>();
}

static void write_image(
    io::BaseByteStream &input_stream,
    const TextureInfo &texture_info,
    res::Image &image,
    const size_t stride)
{
    if (!texture_info.has_data)
        return;

    input_stream.seek(texture_info.texture_offset);
    if (input_stream.read(texture_magic.size()) != texture_magic)
        throw err::CorruptDataError("Corrupt texture data");
    input_stream.skip(2);
    const auto format = input_stream.read_le<u16>();
    const auto width = input_stream.read_le<u16>();
    const auto height = input_stream.read_le<u16>();
    const auto data_size = input_stream.read_le<u32>();
    auto data = input_stream.read(data_size);
    auto data_ptr = data.get<const u8>();

    for (const auto y : algo::range(height))
    for (const auto x : algo::range(width))
    {
        res::Pixel color;
        switch (format)
        {
            case 1:
                color = res::read_pixel<res::PixelFormat::BGRA8888>(data_ptr);
                break;

            case 3:
                color = res::read_pixel<res::PixelFormat::BGR565>(data_ptr);
                break;

            case 5:
                color = res::read_pixel<res::PixelFormat::BGRA4444>(data_ptr);
                break;

            case 7:
                color = res::read_pixel<res::PixelFormat::Gray8>(data_ptr);
                break;

            default:
                throw err::NotSupportedError(algo::format(
                    "Unknown color format: %d", format));
        }

        image.at(x + texture_info.x, y + texture_info.y) = color;
    }
}

static std::vector<TextureInfo> read_texture_info_list(
    io::BaseByteStream &input_stream)
{
    std::vector<TextureInfo> texture_info_list;
    u32 base_offset = 0;
    while (true)
    {
        TextureInfo texture_info;

        input_stream.seek(base_offset);
        input_stream.skip(8);
        const auto use_old = input_stream.read_le<u32>() == 0;

        input_stream.seek(base_offset);
        const auto next_offset = use_old
            ? read_old_texture_info(texture_info, input_stream, base_offset)
            : read_new_texture_info(texture_info, input_stream, base_offset);

        if (texture_info.has_data)
            if (texture_info.width > 0 && texture_info.height > 0)
                texture_info_list.push_back(texture_info);

        if (next_offset == base_offset)
            break;
        base_offset = next_offset;
    }
    return texture_info_list;
}

algo::NamingStrategy AnmArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Root;
}

bool AnmArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("anm");
}

std::unique_ptr<dec::ArchiveMeta> AnmArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto texture_info_list = read_texture_info_list(input_file.stream);

    std::map<std::string, std::vector<TextureInfo>> map;
    for (const auto &texture_info : texture_info_list)
        map[texture_info.name].push_back(texture_info);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto &kv : map)
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = kv.first;
        entry->texture_info_list = kv.second;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> AnmArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    size_t width = 0;
    size_t height = 0;
    for (const auto &texture_info : entry->texture_info_list)
    {
        input_file.stream.peek(texture_info.texture_offset, [&]()
        {
            input_file.stream.skip(texture_magic.size());
            input_file.stream.skip(2);
            input_file.stream.skip(2);
            const auto chunk_width = input_file.stream.read_le<u16>();
            const auto chunk_height = input_file.stream.read_le<u16>();
            width = std::max(width, texture_info.x + chunk_width);
            height = std::max(height, texture_info.y + chunk_height);
        });
    }

    res::Image image(width, height);
    for (const auto &texture_info : entry->texture_info_list)
        write_image(input_file.stream, texture_info, image, width);

    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<AnmArchiveDecoder>(
    "team-shanghai-alice/anm");
