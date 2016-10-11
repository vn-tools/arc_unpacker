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

#include "dec/bgi/dsc_file_decoder.h"
#include "algo/range.h"
#include "dec/bgi/common.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::bgi;

namespace
{
    struct NodeInfo final
    {
        bool has_children;
        bool look_behind;
        u8 value;
        u32 children[2];
    };

    using NodeList = std::vector<std::unique_ptr<NodeInfo>>;
}

static const bstr magic = "DSC FORMAT 1.00\x00"_b;

static int is_image(const bstr &input)
{
    io::MemoryByteStream input_stream(input);
    const auto width = input_stream.read_le<u16>();
    const auto height = input_stream.read_le<u16>();
    const auto bpp = input_stream.read<u8>();
    const auto zeros = input_stream.read(11);
    for (const auto i : algo::range(zeros.size()))
        if (zeros[i])
            return false;
    return width && height && (bpp == 8 || bpp == 24 || bpp == 32);
}

static NodeList get_nodes(io::BaseByteStream &input_stream, u32 key)
{
    NodeList nodes;
    for (const auto i : algo::range(1024))
    {
        auto node_info = std::make_unique<NodeInfo>();
        node_info->has_children = false;
        node_info->value = 0;
        nodes.push_back(std::move(node_info));
    }

    std::vector<u32> arr0;
    for (const auto n : algo::range(512))
    {
        u8 tmp = input_stream.read<u8>() - get_and_update_key(key);
        if (tmp)
            arr0.push_back((tmp << 16) + n);
    }

    std::sort(arr0.begin(), arr0.end());

    size_t arr0_pos;
    u32 n = 0, unk0 = 0x200, unk1 = 1, node_index = 1;
    u32 arr1[1024] = {0};
    u32 *node_ptr = arr1;
    for (arr0_pos = 0; arr0_pos < arr0.size(); n++)
    {
        u32 *arr1_ptr = &arr1[unk0];
        u32 *arr1_old_ptr = arr1_ptr;
        u32 group_count = 0;

        while (true)
        {
            const u32 c = arr0_pos < arr0.size() ? arr0[arr0_pos] : 0;
            if (n != (c >> 16))
                break;
            nodes[*node_ptr]->has_children = false;
            nodes[*node_ptr]->look_behind = (arr0[arr0_pos] & 0x100) != 0;
            nodes[*node_ptr]->value = arr0[arr0_pos] & 0xFF;
            arr0_pos++;
            node_ptr++;
            group_count++;
        }

        const u32 unk3 = 2 * (unk1 - group_count);
        if (group_count < unk1)
        {
            unk1 = unk1 - group_count;
            for (const auto i : algo::range(unk1))
            {
                nodes[*node_ptr]->has_children = true;
                for (const auto j : algo::range(2))
                    *arr1_ptr++ = nodes[*node_ptr]->children[j] = node_index++;
                node_ptr++;
            }
        }
        unk1 = unk3;
        node_ptr = arr1_old_ptr;
        unk0 ^= 0x200;
    }
    return nodes;
}

static bstr decompress(
    io::BaseByteStream &input_stream,
    const NodeList &nodes,
    size_t output_size)
{
    bstr output(output_size);
    u8 *output_ptr = output.get<u8>();
    const u8 *output_start = output_ptr;
    const u8 *output_end = output_ptr + output.size();
    io::MsbBitStream bit_stream(input_stream.read_to_eof());

    u32 bits = 0, bit_count = 0;
    while (output_ptr < output_end)
    {
        u32 node_index = 0;
        while (nodes[node_index]->has_children)
            node_index = nodes[node_index]->children[bit_stream.read(1)];

        if (nodes[node_index]->look_behind)
        {
            auto offset = bit_stream.read(12);
            size_t repetitions = nodes[node_index]->value + 2;
            u8 *look_behind = output_ptr - offset - 2;
            if (look_behind < output_start)
                break;
            if (look_behind + repetitions >= output_end)
                break;
            while (repetitions-- && output_ptr < output_end)
                *output_ptr++ = *look_behind++;
        }
        else
        {
            *output_ptr++ = nodes[node_index]->value;
        }
    }

    return output;
}

bool DscFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> DscFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto key = input_file.stream.read_le<u32>();
    const auto output_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);

    const auto nodes = get_nodes(input_file.stream, key);
    const auto data = decompress(
        input_file.stream, std::move(nodes), output_size);

    if (is_image(data))
    {
        io::MemoryByteStream data_stream(data);
        const auto width = data_stream.read_le<u16>();
        const auto height = data_stream.read_le<u16>();
        const auto bpp = data_stream.read<u8>();
        data_stream.skip(11);

        res::PixelFormat fmt;
        switch (bpp)
        {
            case 8:
                fmt = res::PixelFormat::Gray8;
                break;
            case 24:
                fmt = res::PixelFormat::BGR888;
                break;
            case 32:
                fmt = res::PixelFormat::BGRA8888;
                break;
            default:
                throw err::UnsupportedBitDepthError(bpp);
        }
        res::Image image(width, height, data_stream.read_to_eof(), fmt);
        const auto encoder = enc::png::PngImageEncoder();
        return encoder.encode(logger, image, input_file.path);
    }

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write(data);
    output_file->path = input_file.path;
    if (!output_file->path.has_extension())
        output_file->path.change_extension("dat");
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<DscFileDecoder>("bgi/dsc");
