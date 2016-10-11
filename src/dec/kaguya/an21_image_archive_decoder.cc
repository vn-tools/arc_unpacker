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

#include "dec/kaguya/an21_image_archive_decoder.h"
#include "algo/range.h"
#include "dec/kaguya/common/rle.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "AN21"_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        bstr data;
        size_t x, y;
        size_t width, height;
        size_t channels;
    };
}

algo::NamingStrategy An21ImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

bool An21ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> An21ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto unk_count = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    for (const auto i : algo::range(unk_count))
    {
        const auto control = input_file.stream.read<u8>();
        if (control == 0) continue;
        else if (control == 1) input_file.stream.skip(8);
        else if (control == 2) input_file.stream.skip(4);
        else if (control == 3) input_file.stream.skip(4);
        else if (control == 4) input_file.stream.skip(4);
        else if (control == 5) input_file.stream.skip(4);
        else throw err::NotSupportedError("Unsupported control");
    }

    const auto unk2_count = input_file.stream.read_le<u16>();
    input_file.stream.skip(unk2_count * 8);
    if (input_file.stream.read(7) != "[PIC]10"_b)
        throw err::NotSupportedError("Unexpected magic");
    auto meta = std::make_unique<dec::ArchiveMeta>();
    const auto file_count = input_file.stream.read_le<u16>();
    if (!file_count)
        return meta;
    const auto base_x = input_file.stream.read_le<u32>();
    const auto base_y = input_file.stream.read_le<u32>();
    const auto base_width = input_file.stream.read_le<u32>();
    const auto base_height = input_file.stream.read_le<u32>();

    auto base_entry = std::make_unique<CustomArchiveEntry>();
    base_entry->x = input_file.stream.read_le<u32>();
    base_entry->y = input_file.stream.read_le<u32>();
    base_entry->width = input_file.stream.read_le<u32>();
    base_entry->height = input_file.stream.read_le<u32>();
    base_entry->channels = input_file.stream.read_le<u32>();
    base_entry->data = input_file.stream.read(
        base_entry->channels * base_entry->width * base_entry->height);
    auto last_entry = base_entry.get();
    meta->entries.push_back(std::move(base_entry));

    for (const auto i : algo::range(1, file_count))
    {
        const auto bands = input_file.stream.read<u8>();
        const auto size_comp = input_file.stream.read_le<u32>();
        const auto size_orig = last_entry->channels
            * (last_entry->x + last_entry->width)
            * (last_entry->y + last_entry->height);

        auto output = common::decompress_rle(
            input_file.stream.read(size_comp), size_orig, bands);

        for (const auto i : algo::range(output.size()))
            output[i] += last_entry->data[i];

        auto sub_entry = std::make_unique<CustomArchiveEntry>();
        sub_entry->x = last_entry->x;
        sub_entry->y = last_entry->y;
        sub_entry->width = last_entry->width;
        sub_entry->height = last_entry->height;
        sub_entry->channels = last_entry->channels;
        sub_entry->data = output;
        last_entry = sub_entry.get();
        meta->entries.push_back(std::move(sub_entry));
    }

    return meta;
}

std::unique_ptr<io::File> An21ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::Image image(
        entry->width,
        entry->height,
        entry->data,
        entry->channels == 3
            ? res::PixelFormat::BGR888
            : res::PixelFormat::BGRA8888);
    image.flip_vertically();
    return enc::png::PngImageEncoder().encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<An21ImageArchiveDecoder>("kaguya/an21");
