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

#include "dec/playstation/gxt_image_archive_decoder.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::playstation;

static const bstr magic = "GXT\x00"_b;

namespace
{
    enum class TextureBaseFormat : u32
    {
        Pvrt_1_2 = 0x80'00'00'00,
        Pvrt_1_4 = 0x81'00'00'00,
        Pvrt_2_2 = 0x82'00'00'00,
        Pvrt_2_4 = 0x83'00'00'00,
        Dxt_1_4  = 0x86'00'00'00,
        Dxt_3_8  = 0x87'00'00'00,
        Dxt_5_8  = 0x88'00'00'00,
    };

    enum class TextureType : u32
    {
        Swizzled      = 0x00'00'00'00,
        Cube          = 0x40'00'00'00,
        Linear        = 0x60'00'00'00,
        Tiled         = 0x80'00'00'00,
        LinearStrided = 0x0C'00'00'00,
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        int palette_index;
        TextureType texture_type;
        TextureBaseFormat texture_base_format;
        size_t width;
        size_t height;
    };
}

algo::NamingStrategy GxtImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

bool GxtImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> GxtImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version = input_file.stream.read_le<u32>();
    if (version != 0x1000'0003)
        throw err::UnsupportedVersionError(version);

    const auto texture_count = input_file.stream.read_le<u32>();
    const auto texture_data_offset = input_file.stream.read_le<u32>();
    const auto texture_data_size = input_file.stream.read_le<u32>();
    const auto texture_p4_count = input_file.stream.read_le<u32>();
    const auto texture_p8_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(texture_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        entry->palette_index = input_file.stream.read_le<u32>();
        input_file.stream.skip(4);
        entry->texture_type = input_file.stream.read_le<TextureType>();
        entry->texture_base_format
            = input_file.stream.read_le<TextureBaseFormat>();
        entry->width = input_file.stream.read_le<u16>();
        entry->height = input_file.stream.read_le<u16>();
        input_file.stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> GxtImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    if (entry->palette_index != -1)
        throw err::NotSupportedError("Paletted entries are not supported");
    if (entry->texture_type != TextureType::Linear)
        throw err::NotSupportedError("Only linear textures are supported");

    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    res::Image image(
        entry->width, entry->height, data, res::PixelFormat::Gray8);
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<GxtImageArchiveDecoder>(
    "playstation/gxt");
