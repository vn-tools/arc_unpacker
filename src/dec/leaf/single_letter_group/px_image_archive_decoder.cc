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

namespace
{
    struct SharedContext final
    {
        size_t max_block_size;
        size_t image_width;
        size_t image_height;

        std::array<u32, 256> palette;
    };

    struct BlockInfo final
    {
        // these CAN be zero
        const size_t width;
        const size_t height;
        // these CAN be negative
        const int x;
        const int y;

        const u16 type;
        const u16 subtype;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        size_t width;
        size_t height;
        std::vector<uoff_t> block_offsets;
    };
}

static void transcribe_block(
    const SharedContext &context,
    const BlockInfo &block_info,
    const std::vector<u32> &block,
    const size_t max_block_size,
    bstr &output)
{
    for (const auto y : algo::range(block_info.height))
    for (const auto x : algo::range(block_info.width))
    {
        const auto image_x = block_info.x + x;
        const auto image_y = block_info.y + y;

        if (image_x < 0
        || image_y < 0
        || image_x >= static_cast<int>(context.image_width)
        || image_y >= static_cast<int>(context.image_height))
        {
            continue;
        }

        const auto image_idx = image_x + image_y * context.image_width;
        output.get<u32>()[image_idx] = block.at(x + y * context.max_block_size);
    }
}

static void decode_1_8(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    auto output_ptr = output.get<u32>();
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    for (const auto y : algo::range(block_info.height))
    for (const auto x : algo::range(block_info.width))
    {
        const auto tmp = input_stream.read<u8>();
        if (output_ptr < output_end && output_ptr >= output_start)
            *output_ptr++ = 0xFF000000 | (tmp << 16) | (tmp << 8) | tmp;
    }
}

static void decode_1_32(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    auto output_ptr = output.get<u32>();
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    for (const auto y : algo::range(block_info.height))
    for (const auto x : algo::range(block_info.width))
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

static void decode_4_8(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    std::vector<u32> block(context.max_block_size * context.max_block_size);
    size_t block_idx = 0;

    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    while (true)
    {
        const int delta = input_stream.read_le<u32>();
        if (delta == -1)
            break;
        block_idx += delta / 4;

        input_stream.skip(4);

        const auto size = input_stream.read_le<u32>();
        for (const auto i : algo::range(size))
        {
            const auto color = context.palette[input_stream.read<u8>()];
            if (block_idx < block.size())
                block[block_idx] = color | 0xFF000000;
            block_idx++;
        }
    }

    transcribe_block(
        context, block_info, block, context.max_block_size, output);
}

static void decode_4_9(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    std::vector<u32> block(context.max_block_size * context.max_block_size);
    size_t block_idx = 0;

    bool use_alpha = true;
    while (true)
    {
        const int control = input_stream.read_le<u32>();
        if (control == -1)
            break;
        if (control & 0x180000)
            use_alpha = !use_alpha;
        block_idx += (control & 0x1FF) * context.max_block_size;
        block_idx += (control >> 21);

        const auto size = (control >> 9) % context.max_block_size;
        for (const auto i : algo::range(size))
        {
            const auto alpha = use_alpha
                ? (input_stream.read<u8>() << 25) - 0x1000000
                : 0xFF000000;
            const auto tmp = context.palette[input_stream.read<u8>()];
            if (block_idx < block.size())
                block[block_idx] = tmp | alpha;
            block_idx++;
        }
    }

    transcribe_block(
        context, block_info, block, context.max_block_size, output);
}

static void decode_4_32(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    std::vector<u32> block(context.max_block_size * context.max_block_size);
    if (block_info.width > context.max_block_size
    || block_info.height > context.max_block_size)
        throw err::BadDataSizeError();
    size_t block_idx = 0;

    while (true)
    {
        const auto delta = input_stream.read_le<s32>();
        if (delta == -1)
            break;
        if (delta & 0xFF000000)
            continue;
        block_idx += delta / 4;

        input_stream.skip(4);

        const auto size = input_stream.read_le<u32>();
        for (const auto i : algo::range(size))
        {
            const auto tmp = input_stream.read_le<u32>();
            if (block_idx < block.size())
            {
                if (tmp & 0xFF000000)
                {
                    const auto alpha = ((tmp >> 23) + 0xFF) << 24;
                    block[block_idx] = (tmp & 0xFFFFFF) | alpha;
                }
                else
                {
                    block[block_idx] = tmp | 0xFF000000;
                }
            }
            block_idx++;
        }
    }

    transcribe_block(
        context, block_info, block, context.max_block_size, output);
}

static void decode_4_48(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    std::vector<u32> block(context.max_block_size * context.max_block_size);
    size_t block_idx = 0;

    while (true)
    {
        const int delta = input_stream.read_le<u32>();
        if (delta == -1)
            break;
        if (delta & 0xFF000000)
            continue;
        block_idx += delta / 4;

        input_stream.skip(4);

        const auto size = input_stream.read_le<u32>();
        for (const auto i : algo::range(size))
        {
            const auto tmp = input_stream.read_le<u32>();
            input_stream.skip(2);
            if (block_idx < block.size())
            {
                if (tmp & 0xFF000000)
                {
                    const auto alpha = ((tmp >> 23) + 0xFF) << 24;
                    block[block_idx] = (tmp & 0xFFFFFF) | alpha;
                }
                else
                {
                    block[block_idx] = 0x00000000;
                }
            }
            block_idx++;
        }
    }

    transcribe_block(
        context, block_info, block, context.max_block_size, output);
}

static void decode_7(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    auto output_ptr = output.get<u32>();
    const auto output_start = output.get<const u32>();
    const auto output_end = output.end<const u32>();
    std::array<u32, 256> palette;
    for (const auto i : algo::range(256))
        palette[i] = input_stream.read_le<u32>();
    for (const auto y : algo::range(block_info.height))
    for (const auto x : algo::range(block_info.width))
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

static void read_block(
    SharedContext &context,
    const BlockInfo &block_info,
    io::BaseByteStream &input_stream,
    bstr &output)
{
    if (block_info.type == 0)
    {
        for (const auto i : algo::range(256))
            context.palette[i] = input_stream.read_le<u32>();
        return;
    }

    std::function<void(
            SharedContext &,
            const BlockInfo &,
            io::BaseByteStream &,
            bstr &)>
        decoder;

    if (block_info.type == 1)
    {
        if (block_info.subtype == 8)
            decoder = decode_1_8;
        else if (block_info.subtype == 32)
            decoder = decode_1_32;
    }
    else if (block_info.type == 4)
    {
        if (block_info.subtype == 0x08)
            decoder = decode_4_8;
        else if (block_info.subtype == 0x09)
            decoder = decode_4_9;
        else if (block_info.subtype == 0x20)
            decoder = decode_4_32;
        else if (block_info.subtype == 0x30)
            decoder = decode_4_48;
    }
    else if (block_info.type == 7)
        decoder = decode_7;

    if (!decoder)
    {
        throw err::NotSupportedError(
            algo::format(
                "Unknown block type: %d.%x",
                block_info.type,
                block_info.subtype));
    }

    decoder(context, block_info, input_stream, output);
}

static bstr read_blocks(
    SharedContext &context,
    io::BaseByteStream &input_stream,
    const std::vector<uoff_t> &block_offsets)
{
    bstr output(context.image_width * context.image_height * 4);
    for (const auto block_offset : block_offsets)
    {
        input_stream.seek(block_offset);
        const BlockInfo block_info =
        {
            input_stream.read_le<u32>(), // width
            input_stream.read_le<u32>(), // height
            input_stream.read_le<s32>(), // x
            input_stream.read_le<s32>(), // y
            input_stream.read_le<u16>(), // type
            input_stream.read_le<u16>(), // subtype
        };
        input_stream.seek(block_offset + 32);
        read_block(context, block_info, input_stream, output);
    }
    return output;
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

    SharedContext context =
    {
        1024,          // max_block_size
        entry->width,  // image_width
        entry->height, // image_height
        {},            // palette
    };

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
