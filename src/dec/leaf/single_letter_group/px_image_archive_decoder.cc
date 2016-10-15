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

#include "dec/leaf/single_letter_group/px_image_archive_decoder.h"
#include <array>
#include <set>
#include "algo/format.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "LeafAquaPlus"_b;
static const size_t max_block_size = 1024;

namespace
{
    struct DecoderContext final
    {
        size_t image_width, image_height;
        int block_x, block_y; // these CAN be negative
        size_t block_width, block_height;
        size_t block_depth;
        std::array<u32, 256> palette;
    };

    struct ProceedResult final
    {
        size_t position;
        size_t block_size;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        size_t width, height;
        std::vector<uoff_t> block_offsets;
    };
}

static ProceedResult proceed_and_get_block_size(
    DecoderContext &context, size_t &left)
{
    context.block_y += context.block_x / max_block_size;
    context.block_x %= max_block_size;

    ProceedResult result;
    result.block_size = std::min(left, context.image_width - context.block_x);
    if (!result.block_size)
        throw err::BadDataSizeError();
    left -= result.block_size;

    result.position = context.block_x + context.block_y * context.image_width;
    context.block_x += result.block_size;
    return result;
}

static void decode_1_8(
    DecoderContext &context, io::BaseByteStream &input_stream, bstr &output)
{
    auto output_ptr = output.get<u32>();
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    for (const auto y : algo::range(context.block_height))
    for (const auto x : algo::range(context.block_width))
    {
        const auto tmp = input_stream.read<u8>();
        if (output_ptr < output_end && output_ptr >= output_start)
            *output_ptr++ = 0xFF000000 | (tmp << 16) | (tmp << 8) | tmp;
    }
}

static void decode_1_32(
    DecoderContext &context, io::BaseByteStream &input_stream, bstr &output)
{
    auto output_ptr = output.get<u32>();
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    for (const auto y : algo::range(context.block_height))
    for (const auto x : algo::range(context.block_width))
    {
        const auto tmp = input_stream.read_le<u32>();
        if (output_ptr < output_end && output_ptr >= output_start)
        {
            if (tmp & 0xFF000000)
            {
                const auto alpha = ((tmp >> 23) + 0xFF) << 24;
                *output_ptr++ = (tmp & 0xFFFFFF) | alpha;
            }
            else
            {
                *output_ptr++ = tmp | 0xFF000000;
            }
        }
    }
}

static void decode_4_9(
    DecoderContext &context, io::BaseByteStream &input_stream, bstr &output)
{
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    bool use_alpha = true;
    while (true)
    {
        const int control = input_stream.read_le<u32>();
        if (control == -1)
            return;
        if (control & 0x180000)
            use_alpha = !use_alpha;
        context.block_x += control >> 21;
        context.block_y += control % (max_block_size / 2);
        size_t left = (control >> 9) % max_block_size;
        while (left)
        {
            const auto result = proceed_and_get_block_size(context, left);
            auto output_ptr = output.get<u32>() + result.position;
            for (const auto i : algo::range(result.block_size))
            {
                const auto alpha = use_alpha
                    ? (input_stream.read<u8>() << 25) - 0x1000000
                    : 0xFF000000;
                const auto tmp = context.palette[input_stream.read<u8>()];
                if (output_ptr < output_end && output_ptr >= output_start)
                    *output_ptr++ = tmp | alpha;
            }
        }
    }
}

static void decode_4_32(
    DecoderContext &context, io::BaseByteStream &input_stream, bstr &output)
{
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    while (true)
    {
        const int control = input_stream.read_le<u32>();
        if (control == -1)
            return;
        if (control & 0xFF000000)
            continue;
        context.block_x += control >> 2;
        input_stream.skip(4);
        size_t left = input_stream.read_le<u32>();
        while (left)
        {
            const auto result = proceed_and_get_block_size(context, left);
            auto output_ptr = output.get<u32>() + result.position;
            for (const auto i : algo::range(result.block_size))
            {
                const auto tmp = input_stream.read_le<u32>();
                if (output_ptr < output_end && output_ptr >= output_start)
                {
                    if (tmp & 0xFF000000)
                    {
                        const auto alpha = ((tmp >> 23) + 0xFF) << 24;
                        *output_ptr++ = (tmp & 0xFFFFFF) | alpha;
                    }
                    else
                    {
                        *output_ptr++ = tmp | 0xFF000000;
                    }
                }
            }
        }
    }
}

static void decode_4_48(
    DecoderContext &context, io::BaseByteStream &input_stream, bstr &output)
{
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    while (true)
    {
        const int control = input_stream.read_le<u32>();
        if (control == -1)
            return;
        if (control & 0xFF000000)
            continue;
        context.block_x += control >> 2;
        input_stream.skip(4);
        size_t left = input_stream.read_le<u32>();
        while (left)
        {
            const auto result = proceed_and_get_block_size(context, left);
            auto output_ptr = output.get<u32>() + result.position;
            for (const auto i : algo::range(result.block_size))
            {
                const auto tmp = input_stream.read_le<u32>();
                input_stream.skip(1);
                input_stream.skip(1);
                if (output_ptr < output_end && output_ptr >= output_start)
                {
                    if (tmp & 0xFF000000)
                    {
                        const auto alpha = ((tmp >> 23) + 0xFF) << 24;
                        *output_ptr++ = (tmp & 0xFFFFFF) | alpha;
                    }
                    else
                    {
                        *output_ptr++ = 0x00000000;
                    }
                }
            }
        }
    }
}

static void decode_7(
    DecoderContext &context, io::BaseByteStream &input_stream, bstr &output)
{
    auto output_ptr = output.get<u32>();
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    std::array<u32, 256> palette;
    for (const auto i : algo::range(256))
        palette[i] = input_stream.read_le<u32>();
    for (const auto y : algo::range(context.block_height))
    for (const auto x : algo::range(context.block_width))
    {
        const auto tmp = palette[input_stream.read<u8>()];
        if (output_ptr < output_end && output_ptr >= output_start)
        {
            if (tmp & 0xFF000000)
            {
                const auto alpha = ((tmp >> 23) + 0xFF) << 24;
                *output_ptr++ = (tmp & 0xFFFFFF) | alpha;
            }
            else
            {
                *output_ptr++ = tmp | 0xFF000000;
            }
        }
    }
}

static bstr read_blocks(
    DecoderContext &context,
    io::BaseByteStream &input_stream,
    const std::vector<uoff_t> &block_offsets)
{
    bstr data(context.image_width * context.image_height * 4);
    for (const auto block_offset : block_offsets)
    {
        input_stream.seek(block_offset);

        context.block_width = input_stream.read_le<u32>();
        context.block_height = input_stream.read_le<u32>();
        context.block_x = input_stream.read_le<u32>();
        context.block_y = input_stream.read_le<u32>();

        const auto block_type = input_stream.read_le<u16>();
        const auto block_subtype = input_stream.read_le<u16>();

        input_stream.seek(block_offset + 32);

        if (block_type == 0)
        {
            for (const auto i : algo::range(256))
                context.palette[i] = input_stream.read_le<u32>();
        }
        else
        {
            std::function<void(DecoderContext &, io::BaseByteStream &, bstr &)>
                decoder;
            if (block_type == 1)
            {
                if (block_subtype == 8)
                    decoder = decode_1_8;
                else if (block_subtype == 32)
                    decoder = decode_1_32;
            }
            else if (block_type == 4)
            {
                if (block_subtype == 0x09)
                    decoder = decode_4_9;
                else if (block_subtype == 0x20)
                    decoder = decode_4_32;
                else if (block_subtype == 0x30)
                    decoder = decode_4_48;
            }
            else if (block_type == 7)
                decoder = decode_7;

            if (!decoder)
            {
                throw err::NotSupportedError(algo::format(
                    "Unknown block type: %d.%x", block_type, block_subtype));
            }
            decoder(context, input_stream, data);
        }
    }

    return data;
}

static void read_meta(
    io::BaseByteStream &input_stream,
    dec::ArchiveMeta &meta,
    std::set<uoff_t> &known_offsets)
{
    const auto base_offset = input_stream.pos();
    const auto table_offset = input_stream.pos() + 32;
    const auto control = input_stream.seek(base_offset + 16).read_le<u16>();

    if (control & 0x80)
    {
        const auto file_count = input_stream.seek(base_offset).read_le<u32>();
        const auto data_offset = table_offset + file_count * 4;
        for (const auto i : algo::range(file_count))
        {
            input_stream.seek(data_offset
                + input_stream.seek(table_offset + i * 4).read_le<u32>());

            if (known_offsets.find(input_stream.pos()) != known_offsets.end())
                continue;
            known_offsets.insert(input_stream.pos());

            read_meta(input_stream, meta, known_offsets);
        }
        return;
    }

    if (control & 0x40)
    {
        const auto block_count = input_stream.seek(base_offset).read_le<u32>();
        const auto data_offset = table_offset + block_count * 4;
        std::vector<uoff_t> offsets;
        for (const auto i : algo::range(block_count))
        {
            offsets.push_back(data_offset
                + input_stream.seek(table_offset + i * 4).read_le<u32>());
        }
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->width = input_stream.seek(base_offset + 20).read_le<u32>();
        entry->height = input_stream.seek(base_offset + 24).read_le<u32>();
        for (const auto offset : offsets)
            entry->block_offsets.push_back(offset);
        if (!entry->block_offsets.empty())
            meta.entries.push_back(std::move(entry));
        return;
    }

    auto entry = std::make_unique<CustomArchiveEntry>();
    entry->width = input_stream.seek(base_offset + 20).read_le<u32>();
    entry->height = input_stream.seek(base_offset + 24).read_le<u32>();
    entry->block_offsets.push_back(base_offset);
    meta.entries.push_back(std::move(entry));
}

static void read_meta(
    io::BaseByteStream &input_stream,
    dec::ArchiveMeta &meta)
{
    std::set<uoff_t> known_offsets;
    ::read_meta(input_stream, meta, known_offsets);
}

bool PxImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("px");
}

std::unique_ptr<dec::ArchiveMeta> PxImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();
    ::read_meta(input_file.stream, *meta);
    return std::move(meta);
}

std::unique_ptr<io::File> PxImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    DecoderContext context;
    context.image_width = entry->width;
    context.image_height = entry->height;

    const auto data = read_blocks(
        context, input_file.stream, entry->block_offsets);

    res::Image image(
        entry->width, entry->height, data, res::PixelFormat::BGRA8888);
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

algo::NamingStrategy PxImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<PxImageArchiveDecoder>("leaf/px");
